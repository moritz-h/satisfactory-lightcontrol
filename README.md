# SatisfactoryLightControlMod

Mod for the game Satisfactory by Coffee Stain Studios to control lights with ArtNet.

## Light Show Demo

[![light show demo](https://img.youtube.com/vi/rR7Teg_T_gs/mqdefault.jpg)](https://www.youtube.com/watch?v=rR7Teg_T_gs)  
[https://www.youtube.com/watch?v=rR7Teg_T_gs](https://www.youtube.com/watch?v=rR7Teg_T_gs)

The show was programmed with the light control software [DMXControl](https://dmxcontrol.de/):  
[![DMXControl logo](https://dmxcontrol.de/images/logo/dmxc_logo_2018_bright_no_slogan_75px.png)](https://dmxcontrol.de/)

## Building

This mod can be built by adding it as a plugin to the [SatisfactoryModLoader](https://github.com/satisfactorymodding/SatisfactoryModLoader).
More details are available in the [modding documentation](https://docs.ficsit.app/satisfactory-modding/latest/index.html).

## Usage

The actor names of the controlled lights must be defined before a savegame is loaded.
The values can be stored in the config file `<Satisfactory-Install-Dir>/FactoryGame/Configs/LightControl.cfg` or set within the main menu of the game.

The light names need to be determined from a savegame (i.e. using a savegame editor) and must be specified in the following form:
```
{
    "LightActors": [
        "Build_CeilingLight_C_1234567890",
        "Build_FloodlightPole_C_1234567890",
        "Build_FloodlightWall_C_1234567890",
        "Build_StreetLight_C_1234567890"
    ],
    "SML_ModVersion_DoNotChange": "1.0.0"
}
```

The mod receives ArtNet packets over the network on:
- Net: 0
- SubNet: 0
- Universe: 0

The DMX channels are defined according to the following table:

| DMX Channel | Function                                       |
| -----------:| ---------------------------------------------- |
|          1  | Red value of global color slot 0               |
|          2  | Green value of global color slot 0             |
|          3  | Blue value of global color slot 0              |
|          4  | Red value of global color slot 1               |
|          5  | Green value of global color slot 1             |
|          6  | Blue value of global color slot 1              |
|          7  | Red value of global color slot 2               |
|          8  | Green value of global color slot 2             |
|          9  | Blue value of global color slot 2              |
|         10  | Red value of global color slot 3               |
|         11  | Green value of global color slot 3             |
|         12  | Blue value of global color slot 3              |
|         13  | Red value of global color slot 4               |
|         14  | Green value of global color slot 4             |
|         15  | Blue value of global color slot 4              |
|         16  | Red value of global color slot 5               |
|         17  | Green value of global color slot 5             |
|         18  | Blue value of global color slot 5              |
|         19  | Red value of global color slot 6               |
|         20  | Green value of global color slot 6             |
|         21  | Blue value of global color slot 6              |
|         22  | Dimmer of the first light in the list          |
|         23  | Color slot idx of the first light in the list  |
|         24  | Dimmer of the second light in the list         |
|         25  | Color slot idx if the second light in the list |
|        ...  | ...                                            |

DMX value to color slot idx mapping follows the following formula: `clamp(dmxValue / 36, 0, 6)`.
