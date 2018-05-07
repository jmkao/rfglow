package pw.rfg.bladeblewear.activity;

import android.os.Bundle;
import android.support.wearable.activity.WearableActivity;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;

import pw.rfg.bladeblewear.R;
import pw.rfg.bladeblewear.action.SpeedDialMasterListener;
import pw.rfg.bladeblewear.util.ViewGroupUtil;

public class BLEControl extends WearableActivity {

    private static final String TAG = "BLEControl";

    private View mEditColorButton;
    private ViewGroup mMainMenu, mSubMenu;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_blecontrol);

        mMainMenu = findViewById(R.id.mainMenu);
        mSubMenu = findViewById(R.id.colorMenu);

        mEditColorButton = findViewById(R.id.editColorButton);

        mEditColorButton.setOnTouchListener(new SpeedDialMasterListener(mMainMenu, mSubMenu));

        // Enables Always-on
        setAmbientEnabled();
    }

    @Override
    public void onEnterAmbient(Bundle ambientDetails) {
        super.onEnterAmbient(ambientDetails);

        mMainMenu.setVisibility(View.INVISIBLE);
    }

    @Override
    public void onExitAmbient() {
        super.onExitAmbient();

        mMainMenu.setVisibility(View.VISIBLE);
    }

    @Override
    public void onUpdateAmbient() {
        super.onUpdateAmbient();
    }
}
