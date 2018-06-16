package pw.rfg.bladeblewear.menu;

import android.graphics.Color;

public class ButtonModel {
    private String label;
    private Color uiColor;
    private String action;

    private String imgRes;

    public String getLabel() {
        return label;
    }

    public void setLabel(String label) {
        this.label = label;
    }

    public Color getUiColor() {
        return uiColor;
    }

    public void setUiColor(String uiColor) {
        this.uiColor = Color.valueOf(Color.parseColor(uiColor));
    }

    public String getAction() {
        return action;
    }

    public void setAction(String action) {
        this.action = action;
    }

    public String getImgRes() {
        return imgRes;
    }

    public void setImgRes(String imgRes) {
        this.imgRes = imgRes;
    }

    @Override
    public String toString() {
        return "ButtonModel{" +
                "label='" + label + '\'' +
                ", uiColor=" + uiColor +
                ", action='" + action + '\'' +
                ", imgRes='" + imgRes + '\'' +
                '}';
    }
}
