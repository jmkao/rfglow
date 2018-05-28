package pw.rfg.bladeblewear.activity;

import android.Manifest;
import android.content.pm.PackageManager;
import android.databinding.DataBindingUtil;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.wearable.activity.WearableActivity;
import android.view.View;

import org.apache.commons.jexl3.JexlBuilder;
import org.apache.commons.jexl3.JexlContext;
import org.apache.commons.jexl3.JexlEngine;
import org.apache.commons.jexl3.MapContext;

import pw.rfg.bladeblewear.R;
import pw.rfg.bladeblewear.ble.BleMaster;
import pw.rfg.bladeblewear.menu.ButtonModel;
import pw.rfg.bladeblewear.menu.MenuMaster;
import pw.rfg.bladeblewear.databinding.ActivityMainBinding;

public class MainActivity extends WearableActivity {

    private static final String TAG = "MainActivity";

    private ActivityMainBinding binding;
    private MenuMaster menuMaster;
    private BleMaster bleMaster;

    private static JexlEngine jexl;
    private static JexlContext jexlContext;

    private static final int LOCATION_REQUEST_CODE = 39;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        ActivityCompat.requestPermissions(this,
                new String[] {Manifest.permission.ACCESS_COARSE_LOCATION}, LOCATION_REQUEST_CODE);

        binding = DataBindingUtil.setContentView(this, R.layout.activity_main);

        menuMaster = new MenuMaster(binding);

        // Enables Always-on
        setAmbientEnabled();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == LOCATION_REQUEST_CODE) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                bleMaster = new BleMaster(this, binding.mainStatus);

                jexl = new JexlBuilder().cache(16).strict(false).silent(true).create();
                jexlContext = new MapContext();
                jexlContext.set("ble", bleMaster);
            }
        }
    }

    public void jexlButtonAction(View button) {
        if (button.getTag() instanceof ButtonModel) {
            ButtonModel model = (ButtonModel)button.getTag();
            jexl.createExpression(model.getAction()).evaluate(jexlContext);
        }
    }

    @Override
    public void onEnterAmbient(Bundle ambientDetails) {
        super.onEnterAmbient(ambientDetails);

        binding.mainMenu.setVisibility(View.INVISIBLE);
    }

    @Override
    public void onExitAmbient() {
        super.onExitAmbient();

        binding.mainMenu.setVisibility(View.VISIBLE);
    }

    @Override
    public void onUpdateAmbient() {
        super.onUpdateAmbient();
    }

    @Override
    protected void onDestroy() {
        bleMaster.destroy();
    }
}
