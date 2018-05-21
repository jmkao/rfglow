package pw.rfg.bladeblewear.ble;

import android.util.Log;
import android.widget.TextView;

import org.w3c.dom.Text;

public class BleMaster {
    private static final String TAG = "BleMaster";
    private TextView statusView;

    public BleMaster(TextView statusView) {
        this.statusView = statusView;
    }

    public void changeHS(int h, int s) {
        Log.d(TAG, "changeHS(): "+h+", "+s);
    }
}
