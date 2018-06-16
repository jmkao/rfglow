package pw.rfg.bladeblewear.menu;

import android.support.design.widget.FloatingActionButton;
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

        FloatingActionButton[] starButtonArray = {
                binding.starButton0,
                binding.starButton1,
                binding.starButton2,
                binding.starButton3,
                binding.starButton4,
                binding.starButton5,
                binding.starButton6,
                binding.starButton7,
                binding.starButton8,
                binding.starButton9,
                binding.starButton10,
                binding.starButton11
        };

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

        FloatingActionButton[] pastelButtonArray = {
                binding.pastelButton0,
                binding.pastelButton1,
                binding.pastelButton2,
                binding.pastelButton3,
                binding.pastelButton4,
                binding.pastelButton5,
                binding.pastelButton6,
                binding.pastelButton7,
                binding.pastelButton8,
                binding.pastelButton9,
                binding.pastelButton10,
                binding.pastelButton11
        };

        FloatingActionButton[] bookmarkBlackButtonArray = {
                binding.bookmarkBlackButton0,
                binding.bookmarkBlackButton1,
                binding.bookmarkBlackButton2,
                binding.bookmarkBlackButton3,
                binding.bookmarkBlackButton4,
                binding.bookmarkBlackButton5,
                binding.bookmarkBlackButton6,
                binding.bookmarkBlackButton7,
                binding.bookmarkBlackButton8,
                binding.bookmarkBlackButton9,
                binding.bookmarkBlackButton10,
                binding.bookmarkBlackButton11
        };

        FloatingActionButton[] bookmarkWhiteButtonArray = {
                binding.bookmarkWhiteButton0,
                binding.bookmarkWhiteButton1,
                binding.bookmarkWhiteButton2,
                binding.bookmarkWhiteButton3,
                binding.bookmarkWhiteButton4,
                binding.bookmarkWhiteButton5,
                binding.bookmarkWhiteButton6,
                binding.bookmarkWhiteButton7,
                binding.bookmarkWhiteButton8,
                binding.bookmarkWhiteButton9,
                binding.bookmarkWhiteButton10,
                binding.bookmarkWhiteButton11
        };

        FloatingActionButton[] brightnessButtonArray = {
                binding.brightnessButton0,
                binding.brightnessButton1,
                binding.brightnessButton2,
                binding.brightnessButton3,
                binding.brightnessButton4,
                binding.brightnessButton5,
                binding.brightnessButton6,
                binding.brightnessButton7,
                binding.brightnessButton8,
                binding.brightnessButton9,
                binding.brightnessButton10,
                binding.brightnessButton11
        };

        FloatingActionButton[] gearButtonArray = {
                binding.gearButton0,
                binding.gearButton1,
                binding.gearButton2,
                binding.gearButton3,
                binding.gearButton4,
                binding.gearButton5,
                binding.gearButton6,
                binding.gearButton7,
                binding.gearButton8,
                binding.gearButton9,
                binding.gearButton10,
                binding.gearButton11
        };


        starMenu = new SubMenuModel(binding.starColorButton, binding.starMenu, starButtonArray);
        colorMenu = new SubMenuModel(binding.editColorButton, binding.colorMenu, colorButtonArray);
        pastelMenu = new SubMenuModel(binding.pastelColorButton, binding.pastelMenu, pastelButtonArray);
        bookmarkBlackMenu = new SubMenuModel(binding.bookmarkBlackButton, binding.bookmarkBlackMenu, bookmarkBlackButtonArray);
        bookmarkWhiteMenu = new SubMenuModel(binding.bookmarkWhiteButton, binding.bookmarkWhiteMenu, bookmarkWhiteButtonArray);
        brightnessMenu= new SubMenuModel(binding.brightnessButton, binding.brightnessMenu, brightnessButtonArray);
        gearMenu = new SubMenuModel(binding.gearButton, binding.gearMenu, gearButtonArray);

        subMenus.put("starMenu", starMenu);
        subMenus.put("colorMenu", colorMenu);
        subMenus.put("pastelMenu", pastelMenu);
        subMenus.put("bookmarkBlackMenu", bookmarkBlackMenu);
        subMenus.put("bookmarkWhiteMenu", bookmarkWhiteMenu);
        subMenus.put("brightnessMenu", brightnessMenu);
        subMenus.put("gearMenu", gearMenu);
    }
}
