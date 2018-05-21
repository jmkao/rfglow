package pw.rfg.bladeblewear.menu;

import android.util.Log;
import android.view.HapticFeedbackConstants;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;

import pw.rfg.bladeblewear.util.ViewGroupUtil;

public class SpeedDialMasterListener implements View.OnTouchListener {
    private static final String TAG = "SpeedDialMasterListener";

    private ViewGroup parentMenu, subMenu;

    private View lastHover = null;

    public static SpeedDialMasterListener assign(SubMenuModel subMenu, ViewGroup parentMenu) {
        return assign(subMenu.getActivateButton(), parentMenu, subMenu.getMenu());
    }

    public static SpeedDialMasterListener assign(View button, ViewGroup parentMenu, ViewGroup subMenu) {
        SpeedDialMasterListener listener
                = new SpeedDialMasterListener(parentMenu, subMenu);

        button.setOnTouchListener(listener);

        return listener;
    }

    private SpeedDialMasterListener(ViewGroup parentMenu, ViewGroup subMenu) {
        this.parentMenu = parentMenu;
        this.subMenu = subMenu;
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                Log.v(TAG, "DOWN touch action on topButton");
                v.performHapticFeedback(HapticFeedbackConstants.LONG_PRESS);
                parentMenu.setVisibility(View.INVISIBLE);
                subMenu.setVisibility(View.VISIBLE);
                return true;
            case MotionEvent.ACTION_UP:
                Log.v(TAG, "UP touch action on topButton");
                View upView = ViewGroupUtil.findViewInMotion(subMenu, event);
                if (upView != null) {
                    upView.performClick();
                    Log.v(TAG, "subButton UP on "+upView.getTag());
                }
                parentMenu.setVisibility(View.VISIBLE);
                subMenu.setVisibility(View.INVISIBLE);
                return true;
            case MotionEvent.ACTION_MOVE:
                //Log.v(TAG, "MOVE touch action on startButton");
                View hoverView = ViewGroupUtil.findViewInMotion(subMenu, event);
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
