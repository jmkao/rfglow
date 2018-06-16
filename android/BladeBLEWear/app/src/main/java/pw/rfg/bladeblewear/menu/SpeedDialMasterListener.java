package pw.rfg.bladeblewear.menu;

import android.util.Log;
import android.view.HapticFeedbackConstants;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import pw.rfg.bladeblewear.util.ViewGroupUtil;

public class SpeedDialMasterListener implements View.OnTouchListener {
    private static final String TAG = "SpeedDialMasterListener";

    private ViewGroup parentMenu, subMenu;

    private View lastHover = null;

    private TextView buttonLabel;

    public static SpeedDialMasterListener assign(SubMenuModel subMenu, ViewGroup parentMenu, TextView buttonLabel) {
        return assign(subMenu.getActivateButton(), parentMenu, subMenu.getMenu(), buttonLabel);
    }

    public static SpeedDialMasterListener assign(View button, ViewGroup parentMenu, ViewGroup subMenu, TextView buttonLabel) {
        SpeedDialMasterListener listener
                = new SpeedDialMasterListener(parentMenu, subMenu, buttonLabel);

        button.setOnTouchListener(listener);

        return listener;
    }

    private SpeedDialMasterListener(ViewGroup parentMenu, ViewGroup subMenu, TextView buttonLabel) {
        this.parentMenu = parentMenu;
        this.subMenu = subMenu;
        this.buttonLabel = buttonLabel;
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
                buttonLabel.setText("");
                parentMenu.setVisibility(View.VISIBLE);
                subMenu.setVisibility(View.INVISIBLE);
                return true;
            case MotionEvent.ACTION_MOVE:
                //Log.v(TAG, "MOVE touch action on startButton");
                View hoverView = ViewGroupUtil.findViewInMotion(subMenu, event);
                if (hoverView != null) {
                    //Log.v(TAG, "subMenu MOVE on "+hoverView.getTag());
                    hoverView.setPressed(true);
                    if (hoverView.getTag() instanceof ButtonModel) {
                        buttonLabel.setText(((ButtonModel) hoverView.getTag()).getLabel());
                    }
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
                        buttonLabel.setText("");
                        lastHover = null;
                    }
                }
                return true;
            default:
                return false;
        }
    }

}
