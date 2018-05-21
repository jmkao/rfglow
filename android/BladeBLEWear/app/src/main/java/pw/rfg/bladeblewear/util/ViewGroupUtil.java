package pw.rfg.bladeblewear.util;

import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;

import java.util.List;

public class ViewGroupUtil {
    private static final String TAG = "ViewGroupUtil";

    public static View findViewInMotion(ViewGroup viewGroup, MotionEvent event) {
        for (int i=0; i<viewGroup.getChildCount(); i++) {
            View child = viewGroup.getChildAt(i);
            if (isMotionInView(child, event)) {
                return child;
            }
        }
        return null;
    }

    public static View findViewInMotion(List<View> views, MotionEvent event) {
        for (View view : views) {
            if (isMotionInView(view, event)) {
                return view;
            }
        }
        return null;
    }

    private static boolean isMotionInView(View view, MotionEvent event) {
        int x = (int)event.getRawX();
        int y = (int)event.getRawY();

        //Log.v(TAG, "Checking event at ("+x+", "+y+")");
        return isXYInView(view, x, y);
    }

    private static boolean isXYInView(View view, int x, int y) {
        //Log.v(TAG, "Checking child "+view.getTag());
//        Log.v(TAG, "Child bounds: ("+view.getLeft()+", "+view.getTop()+", "
//                + view.getRight() + ", "+view.getBottom()+")");
        if (x > view.getLeft() && x < view.getRight()
                && y > view.getTop() && y < view.getBottom()) {
            return true;
        }
        return false;
    }
}
