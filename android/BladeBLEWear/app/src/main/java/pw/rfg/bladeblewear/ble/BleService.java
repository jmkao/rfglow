package pw.rfg.bladeblewear.ble;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.support.annotation.Nullable;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;
import android.widget.TextView;

import org.w3c.dom.Text;

import java.util.Arrays;
import java.util.List;
import java.util.Set;

public class BleService extends Service {
    private static final String TAG = "BleService";

    private static final String DEVICE_NAME = "RFGLOW";
    private static final ParcelUuid TX_UUID = ParcelUuid.fromString("6e400002-b5a3-f393-e0a9-e50e24dcca9e");
    private static final ParcelUuid SERVICE_UUID = ParcelUuid.fromString("6e400001-b5a3-f393-e0a9-e50e24dcca9e");

    private static final ScanSettings scanSettings =
            new ScanSettings.Builder()
                    .setScanMode(ScanSettings.SCAN_MODE_LOW_POWER)
                    .setMatchMode(ScanSettings.MATCH_MODE_STICKY)
                    //.setCallbackType(ScanSettings.CALLBACK_TYPE_FIRST_MATCH)
                    .setNumOfMatches(ScanSettings.MATCH_NUM_ONE_ADVERTISEMENT)
                    .build();

    private static final ScanFilter scanFilter = new ScanFilter.Builder()
            .setDeviceName(DEVICE_NAME)
            .setServiceUuid(SERVICE_UUID)
            .build();
    private static final ScanFilter[] scanFilterArray = {scanFilter};
    private static final List<ScanFilter> scanFilters = Arrays.asList(scanFilterArray);

    private String status = "Instantiated";

    private BluetoothManager bleManager = null;
    private BluetoothAdapter bleAdapter = null;
    private BluetoothGatt bleGatt = null;
    private BluetoothLeScanner bleScanner = null;
    private BluetoothGattService bleService = null;
    private BluetoothGattCharacteristic bleTX = null;

    private final LocalBroadcastManager lbm = LocalBroadcastManager.getInstance(this);

    private ScanCallback failOnlyScanCallback = new ScanCallback() {
        @Override
        public void onScanFailed(int errorCode) {
            Log.w(TAG, "cleanup() stopScan failed with code: "+errorCode);
        }
    };

    private ScanCallback scanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            BluetoothDevice device = result.getDevice();
            Log.d(TAG, "BLE onScanResult() for device: "+device.getName());
            Log.d(TAG, "onScanResult callbackType: "+callbackType);
            if (!DEVICE_NAME.equals(device.getName())) {
                return;
            }
            updateBleStatus("Device found");
            if (bleScanner != null) {
                Log.d(TAG, "onScanResult() Stopping BLE Scan");
                bleScanner.stopScan(scanCallback);
                bleScanner.flushPendingScanResults(scanCallback);
            }
            if (bleGatt != null) {
                Log.w(TAG, "bleGatt exists in onScanResult() with state: "
                        +bleManager.getConnectionState(bleGatt.getDevice(),BluetoothProfile.GATT));
                return;
            }
            bleGatt = device.connectGatt(BleService.this, false, gattCallback);
        }

        @Override
        public void onScanFailed(int errorCode) {
            Log.w(TAG, "BLE scan failed with code: "+errorCode);
            //initBle();
            return;
        }
    };

    private BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                updateBleStatus("Polling device");
                gatt.discoverServices();
            }

            if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                initBle();
                return;
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            bleService = gatt.getService(SERVICE_UUID.getUuid());
            if (bleService.getCharacteristic(TX_UUID.getUuid()) != null ) {
                Log.d(TAG, "Found TX characteristic in service.");
                bleTX = bleService.getCharacteristic(TX_UUID.getUuid());
                updateBleStatus("Connected");
            } else {
                Log.w(TAG, "BLE GATT service didn't contain expected characteristic.");
                initBle();
                return;
            }
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            Log.d(TAG, "BLE characteristic write complete to "+gatt.getDevice().getName());
        }
    };

    public void sendData(byte data[]) {
        if (!status.equals("Connected")) {
            Log.i(TAG, "sendData() called while not connected - ignoring.");
            return;
        }
        if (data.length != 4) {
            Log.e(TAG, "sendData() received "+data.length+" bytes instead of 4");
            return;
        }
        bleTX.setValue(data);
        bleGatt.writeCharacteristic(bleTX);
        Log.d(TAG, "sendData() sent");
    }

    public BleService() {
    }

    private void initBle() {
        if (bleAdapter == null) {
            Log.d(TAG, "initBle() no Bluetooth adapter - do nothing.");
            return;
        }
        Log.d(TAG, "initBle()");
        cleanup();
        this.updateBleStatus("Initializing");
        bleScanner = bleAdapter.getBluetoothLeScanner();
        this.updateBleStatus("Scanning");
        bleScanner.startScan(scanFilters, scanSettings, scanCallback);
        //bleScanner.startScan(scanCallback);
    }

    private void cleanup() {

        if (bleScanner != null) {
            bleScanner.stopScan(failOnlyScanCallback);
            bleScanner = null;
        }

        if (bleGatt != null) {
            bleGatt.disconnect();
            bleGatt.close();
            bleGatt = null;
        }

        if (bleManager != null) {
            List<BluetoothDevice> devices = bleManager.getConnectedDevices(BluetoothProfile.GATT);
            for (BluetoothDevice device : devices) {
                if (device.getName().equals(DEVICE_NAME)) {
                    BluetoothGatt gatt = device.connectGatt(this, false, new BluetoothGattCallback() {});
                    gatt.disconnect();
                    gatt.close();
                }
            }
        }
    }

    private void updateBleStatus(String status) {
        this.status = status;
        lbm.sendBroadcast(new Intent("bleStatus").putExtra("status", this.status));
    }

    @Override
    public void onCreate() {
        super.onCreate();
    }

    @Override
    public void onDestroy() {
        cleanup();
    }

    public class LocalBinder extends Binder {
        BleService getService() {
            return BleService.this;
        }
    }
    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind()");

        bleManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        if (bleManager != null) {
            bleAdapter = bleManager.getAdapter();
            if (bleAdapter == null || !bleAdapter.isEnabled()) {
                bleAdapter = null;
            }
        }

        initBle();

        return new LocalBinder();
    }

    @Override
    public boolean onUnbind(Intent intent) {
        Log.d(TAG, "onUnbind()");
        cleanup();
        return false;
    }
}
