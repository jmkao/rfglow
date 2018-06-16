package pw.rfg.bladeblewear.menu;

import android.content.res.ColorStateList;
import android.media.Image;
import android.support.design.widget.FloatingActionButton;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.google.common.io.Resources;

import java.util.Arrays;
import java.util.List;

public class SubMenuModel {
    private static final String TAG = "SubMenuModel";

    private View activateButton;
    private ViewGroup menu;

    public ImageView[] buttons;
    public List<? extends ImageView> buttonList;

    public List<ButtonModel> models;

    public SubMenuModel() {

    }

    public SubMenuModel(View activateButton, ViewGroup menu, ImageView[] buttons) {
        this.activateButton = activateButton;
        this.menu = menu;
        this.buttons = buttons;
        this.buttonList = Arrays.asList(buttons);
    }

    public void loadModels(SubMenuModel fromMenu) {
        setModels(fromMenu.getModels());
        updateButtonFromModel();
        Log.d(TAG, "Updated "+this.toString());
    }

    private void updateButtonFromModel() {
        for (int i=0; i<models.size(); i++) {
            ButtonModel model = models.get(i);
            ImageView button = buttons[i];

            button.setTag(model);
            //button.setBackgroundColor(model.getUiColor().toArgb());
            button.setColorFilter(model.getUiColor().toArgb());
            button.setBackgroundTintList(ColorStateList.valueOf(model.getUiColor().toArgb()));

            if (model.getImgRes() != null && !model.getImgRes().isEmpty()) {
                int resId = button.getContext().getResources().getIdentifier(model.getImgRes(), "drawable", button.getContext().getPackageName());
                if (resId > 0) {
                    button.setImageResource(resId);
                }
            }

            //TODO load drawable from resource name and set into button.setImageDrawable
        }
    }

    public View getActivateButton() {
        return activateButton;
    }

    public void setActivateButton(View activateButton) {
        this.activateButton = activateButton;
    }

    public ViewGroup getMenu() {
        return menu;
    }

    public void setMenu(ViewGroup menu) {
        this.menu = menu;
    }

    public List<ButtonModel> getModels() {
        return models;
    }

    public void setModels(List<ButtonModel> models) {
        this.models = models;
    }

    @Override
    public String toString() {
        return "SubMenuModel{" +
                "models=" + models +
                '}';
    }
}
