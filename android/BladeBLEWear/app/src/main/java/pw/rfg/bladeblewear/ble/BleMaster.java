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
    private Activity mainActivity;
    private TextView statusView;
    private BleService bleService;

    private int h, s, v = 128;

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
        byte[] payload = new byte[4];

        payload[0] = (byte) ((h >> 8) & 0x00FF);
        payload[1] = (byte) (h & 0x00FF);
        payload[2] = (byte) s;
        payload[3] = (byte) v;

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

    public void changeHS(int h, int s) {
        Log.d(TAG, "changeHS(): "+h+", "+s);
        this.h = h;
        this.s = s;
        sendUpdate();
    }
}
