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

    private ViewGroup parentView, subView;

    private View lastHover = null;

    private TextView buttonLabel;

    private SubMenuModel subMenu;

    public static SpeedDialMasterListener assign(SubMenuModel subMenu, ViewGroup parentView, TextView buttonLabel) {
        SpeedDialMasterListener listener = new SpeedDialMasterListener(parentView, subMenu.getView(), buttonLabel, subMenu);
        subMenu.getActivateButton().setOnTouchListener(listener);

        return listener;
    }

    private SpeedDialMasterListener(ViewGroup parentView, ViewGroup subView, TextView buttonLabel, SubMenuModel subMenu) {
        this.parentView = parentView;
        this.subView = subView;
        this.buttonLabel = buttonLabel;
        this.subMenu = subMenu;
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                Log.v(TAG, "DOWN touch action on topButton");
                v.performHapticFeedback(HapticFeedbackConstants.LONG_PRESS);
                subMenu.updateButtonsFromModel();
                parentView.setElevation(0);
                subView.setElevation(1);
                //parentView.setVisibility(View.INVISIBLE);
                //subView.setVisibility(View.VISIBLE);
                return true;
            case MotionEvent.ACTION_UP:
                Log.v(TAG, "UP touch action on topButton");
                View upView = ViewGroupUtil.findViewInMotion(subView, event);
                if (upView != null) {
                    upView.performClick();
                    Log.v(TAG, "subButton UP on "+upView.getTag());
                }
                buttonLabel.setText("");
                parentView.setElevation(1);
                subView.setElevation(0);
                //parentView.setVisibility(View.VISIBLE);
                //subView.setVisibility(View.INVISIBLE);
                return true;
            case MotionEvent.ACTION_MOVE:
                //Log.v(TAG, "MOVE touch action on startButton");
                View hoverView = ViewGroupUtil.findViewInMotion(subView, event);
                if (hoverView != null) {
                    //Log.v(TAG, "subView MOVE on "+hoverView.getTag());
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
