package pw.rfg.bladeblewear.util;

import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;

public class ViewGroupUtil {
    private static final String TAG = "ViewGroupUtil";

    public static View findViewInXY(ViewGroup viewGroup, MotionEvent event) {
        int x = (int)event.getRawX();
        int y = (int)event.getRawY();

        Log.v(TAG, "Checking event at ("+x+", "+y+")");
        for (int i=0; i<viewGroup.getChildCount(); i++) {
            View child = viewGroup.getChildAt(i);
            Log.v(TAG, "Checking child "+child.getTag().toString());
            Log.v(TAG, "Child bounds: ("+child.getLeft()+", "+child.getTop()+", "
            + child.getRight() + ", "+child.getBottom()+")");
            if (x > child.getLeft() && x < child.getRight()
                    && y > child.getTop() && y < child.getBottom()) {
                return child;
            }
        }
        return null;
    }
}
