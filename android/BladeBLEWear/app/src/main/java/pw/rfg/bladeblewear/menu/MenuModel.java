package pw.rfg.bladeblewear.menu;

import com.google.android.material.floatingactionbutton.FloatingActionButton;
import android.view.ViewGroup;
import android.widget.TextView;

import java.util.HashMap;

import pw.rfg.bladeblewear.databinding.ActivityMainBinding;

public class MenuModel {
    ActivityMainBinding binding;

    public ViewGroup topMenu;

    public SubMenuModel colorMenu, starMenu, pastelMenu, bookmarkBlackMenu, bookmarkWhiteMenu,
            brightnessMenu, gearMenu;

    public TextView buttonLabel;

    public HashMap<String, SubMenuModel> subMenus = new HashMap<>();

    public MenuModel(ActivityMainBinding binding) {
        this.binding = binding;

        this.buttonLabel = binding.buttonLabel;

        topMenu = binding.mainMenu;

        FloatingActionButton[] colorButtonArray = {
                binding.colorButton0,
                binding.colorButton1,
                binding.colorButton2,
                binding.colorButton3,
                binding.colorButton4,
                binding.colorButton5,
                binding.colorButton6,
                binding.colorButton7,
                binding.colorButton8,
                binding.colorButton9,
                binding.colorButton10,
                binding.colorButton11
        };

        starMenu = new SubMenuModel(binding.starColorButton, binding.colorMenu, colorButtonArray);
        colorMenu = new SubMenuModel(binding.editColorButton, binding.colorMenu, colorButtonArray);
        pastelMenu = new SubMenuModel(binding.pastelColorButton, binding.colorMenu, colorButtonArray);
        bookmarkBlackMenu = new SubMenuModel(binding.bookmarkBlackButton, binding.colorMenu, colorButtonArray);
        bookmarkWhiteMenu = new SubMenuModel(binding.bookmarkWhiteButton, binding.colorMenu, colorButtonArray);
        brightnessMenu= new SubMenuModel(binding.brightnessButton, binding.colorMenu, colorButtonArray);
        gearMenu = new SubMenuModel(binding.gearButton, binding.colorMenu, colorButtonArray);

        subMenus.put("starMenu", starMenu);
        subMenus.put("colorMenu", colorMenu);
        subMenus.put("pastelMenu", pastelMenu);
        subMenus.put("bookmarkBlackMenu", bookmarkBlackMenu);
        subMenus.put("bookmarkWhiteMenu", bookmarkWhiteMenu);
        subMenus.put("brightnessMenu", brightnessMenu);
        subMenus.put("gearMenu", gearMenu);
    }
}
