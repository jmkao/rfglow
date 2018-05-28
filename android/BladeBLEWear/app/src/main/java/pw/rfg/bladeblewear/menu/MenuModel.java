package pw.rfg.bladeblewear.menu;

import android.support.design.widget.FloatingActionButton;
import android.view.ViewGroup;

import java.util.HashMap;

import pw.rfg.bladeblewear.databinding.ActivityMainBinding;

public class MenuModel {
    ActivityMainBinding binding;

    public ViewGroup topMenu;

    public SubMenuModel colorMenu;

    public HashMap<String, SubMenuModel> subMenus = new HashMap<>();

    public MenuModel(ActivityMainBinding binding) {
        this.binding = binding;

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

        colorMenu = new SubMenuModel(binding.editColorButton, binding.colorMenu, colorButtonArray);
        subMenus.put("colorMenu", colorMenu);
    }
}
