package pw.rfg.bladeblewear.action;

import android.util.Log;
import android.view.HapticFeedbackConstants;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;

import pw.rfg.bladeblewear.util.ViewGroupUtil;

public class SpeedDialMasterListener implements View.OnTouchListener {
    private static final String TAG = "SpeedDialMasterListener";

    private ViewGroup topMenu, subMenu;

    private View lastHover = null;

    public SpeedDialMasterListener(ViewGroup topMenu, ViewGroup subMenu) {
        this.topMenu = topMenu;
        this.subMenu = subMenu;
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                Log.d(TAG, "DOWN touch action on topButton");
                v.performHapticFeedback(HapticFeedbackConstants.LONG_PRESS);
                topMenu.setVisibility(View.INVISIBLE);
                subMenu.setVisibility(View.VISIBLE);
                return true;
            case MotionEvent.ACTION_UP:
                Log.d(TAG, "UP touch action on topButton");
                View upView = ViewGroupUtil.findViewInXY(subMenu, event);
                if (upView != null) {
                    upView.performClick();
                    Log.d(TAG, "subButton UP on "+upView.getTag());
                }
                topMenu.setVisibility(View.VISIBLE);
                subMenu.setVisibility(View.INVISIBLE);
                return true;
            case MotionEvent.ACTION_MOVE:
                //Log.v(TAG, "MOVE touch action on startButton");
                View hoverView = ViewGroupUtil.findViewInXY(subMenu, event);
                if (hoverView != null) {
                    //Log.v(TAG, "subMenu MOVE on "+hoverView.getTag());
                    hoverView.setPressed(true);
                    if (lastHover != hoverView) {
                        hoverView.performHapticFeedback(HapticFeedbackConstants.KEYBOARD_TAP);
                        if (lastHover != null) {
                            lastHover.setPressed(false);
                        }
                    }
                    lastHover = hoverView;
                } else {
                    if (lastHover != null) {
                        lastHover.setPressed(false);
                        lastHover = null;
                    }
                }
                return true;
            default:
                return false;
        }
    }

}
