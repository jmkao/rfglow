package pw.rfg.bladeblewear.activity;

import android.databinding.DataBindingUtil;
import android.os.Bundle;
import android.support.wearable.activity.WearableActivity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;

import com.google.firebase.firestore.FirebaseFirestore;

import org.apache.commons.jexl3.JexlBuilder;
import org.apache.commons.jexl3.JexlContext;
import org.apache.commons.jexl3.JexlEngine;
import org.apache.commons.jexl3.MapContext;

import pw.rfg.bladeblewear.R;
import pw.rfg.bladeblewear.ble.BleMaster;
import pw.rfg.bladeblewear.menu.ButtonModel;
import pw.rfg.bladeblewear.menu.MenuMaster;
import pw.rfg.bladeblewear.menu.SpeedDialMasterListener;
import pw.rfg.bladeblewear.databinding.ActivityBlecontrolBinding;

public class BLEControl extends WearableActivity {

    private static final String TAG = "BLEControl";

    private ActivityBlecontrolBinding binding;
    private MenuMaster menuMaster;
    private BleMaster bleMaster;

    private static JexlEngine jexl;
    private static JexlContext jexlContext;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = DataBindingUtil.setContentView(this, R.layout.activity_blecontrol);

        menuMaster = new MenuMaster(binding);
        bleMaster = new BleMaster(binding.mainStatus);

        jexl = new JexlBuilder().cache(16).strict(false).silent(true).create();
        jexlContext = new MapContext();
        jexlContext.set("ble", bleMaster);

        // Enables Always-on
        setAmbientEnabled();
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
}
