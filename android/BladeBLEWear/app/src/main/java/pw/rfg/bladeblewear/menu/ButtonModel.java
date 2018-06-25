package pw.rfg.bladeblewear.menu;

import android.graphics.Color;

public class ButtonModel {
    private String label;
    private Color uiColor;
    private String action;
    private int index;
    private String imgRes;
    private Color imgColor;

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

    public Color getImgColor() {
        return imgColor;
    }

    public void setImgColor(String imgColor) {
        this.imgColor = Color.valueOf(Color.parseColor(imgColor));
    }


    public int getIndex() {
        return index;
    }

    public void setIndex(int index) {
        this.index = index;
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
