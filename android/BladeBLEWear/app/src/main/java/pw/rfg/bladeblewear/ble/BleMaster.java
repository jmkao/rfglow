package pw.rfg.bladeblewear.ble;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Handler;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;
import android.widget.TextView;

import org.w3c.dom.Text;

public class BleMaster {
    private static final String TAG = "BleMaster";

    private static final int DEFAULT_BRIGHTNESS = 60;
    private static final int DEFAULT_DIM_LEVEL = 1;

    private Activity mainActivity;
    private TextView statusView;
    private BleService bleService;

    private int h = 0, s = 255, v = DEFAULT_BRIGHTNESS;
    private int dim = DEFAULT_DIM_LEVEL;
    private volatile boolean off = true;

    private int[][] cycleArray = null;
    private int cycleIndex = -1;
    private int cycleFadeMs = 0;
    private int cycleDelayMs = 0;
    private boolean isCycling = false;

    private Handler handler = new Handler();
    private Runnable cycleTask = new Runnable() {
        public void run() {
          handler.removeCallbacks(this);
          if (!isCycling || cycleIndex < 0 || cycleArray == null || BleMaster.this.off) {
              isCycling = false;
              cycleIndex = -1;
              cycleArray = null;
              return;
          }
          cycleIndex++;
          cycleIndex = cycleIndex % cycleArray.length;
          BleMaster.this.h = cycleArray[cycleIndex][0];
          BleMaster.this.s = cycleArray[cycleIndex][1];
          if (cycleArray[cycleIndex].length == 3) {
              BleMaster.this.v = cycleArray[cycleIndex][2];
          }
          BleMaster.this.sendUpdate(cycleFadeMs);
          handler.postDelayed(this, cycleDelayMs);
        }
    };

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
        this.sendUpdate(0);
    }

    private void sendUpdate(int fadeMs) {
        byte[] payload;

        if (this.off) {
            payload = new byte[] {0, 0, 0, 0};
        } else {
            if (fadeMs > 0) {
                payload = new byte[6];
                payload[4] = (byte) ((fadeMs >> 8) & 0x00FF);
                payload[5] = (byte) (fadeMs & 0x00FF);
            } else {
                payload = new byte[4];
            }

            int dimH = this.dim * 360 + h;

            payload[0] = (byte) ((dimH >> 8) & 0x00FF);
            payload[1] = (byte) (dimH & 0x00FF);
            payload[2] = (byte) s;
            payload[3] = (byte) v;
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
        handler.removeCallbacks(cycleTask);
        this.off = true;
        this.isCycling = false;
        this.sendUpdate();
    }

    public void updateReset() {
        handler.removeCallbacks(cycleTask);
        this.dim = DEFAULT_DIM_LEVEL;
        this.v = DEFAULT_BRIGHTNESS;
        this.off = true;
        this.isCycling = false;
        this.updateOff();
    }

    public void setBrightness(int v) {
        this.v = v;

        if (!this.isCycling) {
            this.sendUpdate();
        }
    }

    public void updateHSV(int h, int s, int v) {
        Log.d(TAG, "updateHSV(): "+h+", "+s+", "+v);
        handler.removeCallbacks(cycleTask);
        this.isCycling = false;
        this.off = false;
        this.h = h;
        this.s = s;
        this.v = v;
        this.sendUpdate();
    }

    public void cycleHS(int delayMs, int fadeMs, int[] ... hsArray) {
        handler.removeCallbacks(cycleTask);
        this.off = false;
        this.cycleFadeMs = fadeMs;
        this.cycleDelayMs = delayMs;
        this.cycleArray = hsArray;
        this.cycleIndex = 0;
        this.isCycling = true;

        this.h = hsArray[0][0];
        this.s = hsArray[0][1];
        this.sendUpdate();
        handler.postDelayed(cycleTask, cycleDelayMs);
    }
}
