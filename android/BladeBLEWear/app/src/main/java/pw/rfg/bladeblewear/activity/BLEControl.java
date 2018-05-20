package pw.rfg.bladeblewear.activity;

import android.os.Bundle;
import android.support.wearable.activity.WearableActivity;
import android.view.View;
import android.view.ViewGroup;

import com.google.firebase.firestore.FirebaseFirestore;

import pw.rfg.bladeblewear.R;
import pw.rfg.bladeblewear.action.SpeedDialMasterListener;

public class BLEControl extends WearableActivity {

    private static final String TAG = "BLEControl";

    private View mEditColorButton;
    private ViewGroup mMainMenu, mColorMenu;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_blecontrol);

        mMainMenu = findViewById(R.id.mainMenu);
        mColorMenu = findViewById(R.id.colorMenu);

        mEditColorButton = findViewById(R.id.editColorButton);

        mEditColorButton.setOnTouchListener(new SpeedDialMasterListener(mMainMenu, mColorMenu));

        // Enables Always-on
        setAmbientEnabled();

        FirebaseFirestore db = FirebaseFirestore.getInstance();
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
