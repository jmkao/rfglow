package pw.rfg.bladeblewear.ble;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;
import android.widget.TextView;

import org.w3c.dom.Text;

public class BleMaster {
    private static final String TAG = "BleMaster";

    private static final int DEFAULT_BRIGHTNESS = 120;
    private static final int DEFAULT_DIM_LEVEL = 1;

    private Activity mainActivity;
    private TextView statusView;
    private BleService bleService;

    private int h = 0, s = 255, v = DEFAULT_BRIGHTNESS;
    private int dim = DEFAULT_DIM_LEVEL;
    private boolean off = true;

    private final ServiceConnection bleServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName componentName, IBinder service) {
            bleService = ((BleService.LocalBinder) service).getService();
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            bleService = null;
        }
    };

    private void sendUpdate() {
        byte[] payload = {0, 0, 0, 0};

        if (!this.off) {
            int dimH = this.dim * 360 + h;

            payload[0] = (byte) ((dimH >> 8) & 0x00FF);
            payload[1] = (byte) (dimH & 0x00FF);
            payload[2] = (byte) s;
            payload[3] = (byte) v;
        } else {
            // Do nothing, payload[] is initialized to off values
        }

        bleService.sendData(payload);
    }

    public BleMaster(Activity mainActivity, TextView statusView) {
        this.mainActivity = mainActivity;
        this.statusView = statusView;

        Intent bleInitIntent = new Intent(mainActivity, BleService.class);
        mainActivity.bindService(bleInitIntent, bleServiceConnection, Context.BIND_AUTO_CREATE);

        LocalBroadcastManager.getInstance(mainActivity).registerReceiver(new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String status = intent.getStringExtra("status");
                BleMaster.this.statusView.setText(status);
            }
        }, new IntentFilter("bleStatus"));
    }

    public void destroy() {
        mainActivity.unbindService(bleServiceConnection);
    }

    public void updateDim(int dim) {
        Log.d(TAG, "updateDim(): "+dim);
        this.dim = dim;
        sendUpdate();
    }

    public void updateHS(int h, int s) {
        this.updateHSV(h, s, this.v);
    }

    public void updateOff() {
        this.off = true;
        this.sendUpdate();
    }

    public void updateHSV(int h, int s, int v) {
        Log.d(TAG, "updateHSV(): "+h+", "+s+", "+v);

        this.off = false;
        this.h = h;
        this.s = s;
        this.v = v;
        this.sendUpdate();
    }
}
