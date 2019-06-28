package pw.rfg.bladeblewear.menu;

import androidx.annotation.Nullable;
import android.util.Log;

import com.google.firebase.firestore.DocumentChange;
import com.google.firebase.firestore.EventListener;
import com.google.firebase.firestore.FirebaseFirestore;
import com.google.firebase.firestore.FirebaseFirestoreException;
import com.google.firebase.firestore.FirebaseFirestoreSettings;
import com.google.firebase.firestore.QuerySnapshot;

import pw.rfg.bladeblewear.databinding.ActivityMainBinding;

public class MenuMaster {
    private static final String TAG = "MenuMaster";

    private ActivityMainBinding binding;
    private FirebaseFirestore db;

    private MenuModel menuModel;

    public MenuMaster(ActivityMainBinding binding) {
        this.binding = binding;
        this.menuModel = new MenuModel(binding);

        initTopMenu();
        initFirestore();
    }

    private void initTopMenu() {
        SpeedDialMasterListener.assign(menuModel.colorMenu, menuModel.topMenu, menuModel.buttonLabel);
        SpeedDialMasterListener.assign(menuModel.pastelMenu, menuModel.topMenu, menuModel.buttonLabel);
        SpeedDialMasterListener.assign(menuModel.starMenu, menuModel.topMenu, menuModel.buttonLabel);
        SpeedDialMasterListener.assign(menuModel.bookmarkBlackMenu, menuModel.topMenu, menuModel.buttonLabel);
        SpeedDialMasterListener.assign(menuModel.bookmarkWhiteMenu, menuModel.topMenu, menuModel.buttonLabel);
        SpeedDialMasterListener.assign(menuModel.brightnessMenu, menuModel.topMenu, menuModel.buttonLabel);
        SpeedDialMasterListener.assign(menuModel.gearMenu, menuModel.topMenu, menuModel.buttonLabel);
    }


    private void initFirestore() {
        this.db = FirebaseFirestore.getInstance();

        FirebaseFirestoreSettings settings = new FirebaseFirestoreSettings.Builder()
                .setPersistenceEnabled(true)
                .build();

        db.setFirestoreSettings(settings);

        db.collection("menus")
                .addSnapshotListener(new EventListener<QuerySnapshot>() {
                    @Override
                    public void onEvent(@Nullable QuerySnapshot snapshots,
                                        @Nullable FirebaseFirestoreException e) {
                        if (e != null) {
                            Log.w(TAG, "Listen failed.", e);
                            return;
                        }
                        for (DocumentChange dc : snapshots.getDocumentChanges()) {
                            switch (dc.getType()) {
                                case ADDED:
                                case MODIFIED:
                                    SubMenuModel menu = menuModel.subMenus.get(dc.getDocument().getId());
                                    Log.d(TAG, "Menu update for: "+dc.getDocument().getId());
                                    if (menu != null) {
                                        menu.loadModels(dc.getDocument().toObject(SubMenuModel.class));
                                    }
                                    break;
                                case REMOVED:
                                    Log.d(TAG, "Removed menu: " + dc.getDocument().getId());
                                    break;
                            }
                        }
                    }
                });
    }
}
