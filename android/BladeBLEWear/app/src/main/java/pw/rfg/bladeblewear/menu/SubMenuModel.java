package pw.rfg.bladeblewear.menu;

import android.content.res.ColorStateList;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import java.util.Arrays;
import java.util.List;

public class SubMenuModel {
    private static final String TAG = "SubMenuModel";
    private static final int defaultImgColor = Color.parseColor("#000080");
    private static final int blankImgColor = Color.parseColor("#000000");
    private static final ColorStateList blankTintList = ColorStateList.valueOf(blankImgColor);

    private View activateButton;
    private ViewGroup menu;

    public ImageView[] buttons;
    public List<? extends ImageView> buttonList;

    public List<ButtonModel> models;

    private Drawable blankImg;

    public SubMenuModel() {
    }

    public SubMenuModel(View activateButton, ViewGroup menu, ImageView[] buttons) {
        this.activateButton = activateButton;
        this.menu = menu;
        this.buttons = buttons;
        this.buttonList = Arrays.asList(buttons);
        this.blankImg = activateButton.getContext().getDrawable(android.R.color.transparent);
    }

    public void loadModels(SubMenuModel fromMenu) {
        setModels(fromMenu.getModels());
        //updateButtonFromModel();
        Log.d(TAG, "Updated "+this.toString());
    }

    public void updateButtonsFromModel() {
        for (int i=0; i<buttons.length; i++) {
            buttons[i].setColorFilter(blankImgColor);
            buttons[i].setBackgroundTintList(blankTintList);
            buttons[i].setImageDrawable(blankImg);
            buttons[i].setTag(null);
            buttons[i].setClickable(false);
            buttons[i].setHapticFeedbackEnabled(false);
        }

        if (models == null) {
            return;
        }

        for (int i=0; i<models.size(); i++) {
            ButtonModel model = models.get(i);
            ImageView button;

            try {
                button = buttons[model.getIndex()];
            } catch (ArrayIndexOutOfBoundsException e) {
                Log.d(TAG, "Invalid model from Firestore", e);
                continue;
            }

            button.setTag(model);
            button.setClickable(true);
            button.setHapticFeedbackEnabled(true);
            if (model.getUiColor() != null) {
                button.setColorFilter(model.getUiColor().toArgb());
                button.setBackgroundTintList(ColorStateList.valueOf(model.getUiColor().toArgb()));
            }

            if (model.getImgRes() != null && !model.getImgRes().isEmpty()) {
                int resId = button.getContext().getResources().getIdentifier(model.getImgRes(), "drawable", button.getContext().getPackageName());
                if (resId > 0) {
                    button.setImageDrawable(button.getContext().getDrawable(resId));
                    if (model.getImgColor() != null) {
                        button.setColorFilter(model.getImgColor().toArgb());
                    } else {
                        button.setColorFilter(defaultImgColor);
                    }
                } else {
                    Log.d(TAG, "No resource found: "+model.getImgRes());
                }
            }
        }
    }

    public View getActivateButton() {
        return activateButton;
    }

    public void setActivateButton(View activateButton) {
        this.activateButton = activateButton;
    }

    public ViewGroup getView() {
        return menu;
    }

    public void setView(ViewGroup menu) {
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
