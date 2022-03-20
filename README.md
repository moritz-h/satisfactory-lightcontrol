# SatisfactoryLightControlMod

Mod for the game [Satisfactory](https://www.satisfactorygame.com/) by Coffee Stain Studios to control lights with [Art-Net](https://en.wikipedia.org/wiki/Art-Net).

## Light Show Demo

[![light show demo](https://img.youtube.com/vi/rR7Teg_T_gs/mqdefault.jpg)](https://www.youtube.com/watch?v=rR7Teg_T_gs)  
[https://www.youtube.com/watch?v=rR7Teg_T_gs](https://www.youtube.com/watch?v=rR7Teg_T_gs)

The show was programmed with the light control software [DMXControl](https://dmxcontrol.de/):  
[![DMXControl logo](https://dmxcontrol.de/images/logo/dmxc_logo_2018_bright_no_slogan_75px.png)](https://dmxcontrol.de/)

## Building

This mod can be built by adding it as a plugin to the [SatisfactoryModLoader](https://github.com/satisfactorymodding/SatisfactoryModLoader).
More details are available in the [modding documentation](https://docs.ficsit.app/satisfactory-modding/latest/index.html).

## Usage

This mod adds an "Art-Net Lights Control Panel" building to the game.
It is available in the AWESOME Shop when the regular Lights Control Panel is unlocked.
The control panel will receive Art-Net data from an external light control software.
All lights connected to the control panel can be configured within the control panel UI:

![Art-Net Lights Control Panel UI](Docs/ControlPanelUI.png)

At the top are general settings to configure the Net and Sub-Net where the external light control software sends Art-Net data.
Below the start address of the 7 global color channels can be configured, as well as a default start address which will be set as the initial value to all newly connected lights.

Please note, that the 7 color slots are a global game setting and shared between all lights and lights control panels.
A detailed description of the DMX channels can be found in the table below.

Below the general settings is a list of all lights connected to the current Art-Net Lights Control Panel.
On the left is a text box, where it is possible to give each light a meaningful description.
Next to it is a checkbox. If it is checked the light will start blinking.
This allows to easily find a connection between the actual light and the name in the list.
On the right, the start address of each light can be configured. Each light has two channels, a dimmer and a color slot index.
A detailed description is in the table below.

### Color Channel Table

| Channel | Function                        |
| -------:| ------------------------------- |
|      1  | Color Slot 1 - Red (0 - 100%)   |
|      2  | Color Slot 1 - Green (0 - 100%) |
|      3  | Color Slot 1 - Blue (0 - 100%)  |
|      4  | Color Slot 2 - Red (0 - 100%)   |
|      5  | Color Slot 2 - Green (0 - 100%) |
|      6  | Color Slot 2 - Blue (0 - 100%)  |
|      7  | Color Slot 3 - Red (0 - 100%)   |
|      8  | Color Slot 3 - Green (0 - 100%) |
|      9  | Color Slot 3 - Blue (0 - 100%)  |
|     10  | Color Slot 4 - Red (0 - 100%)   |
|     11  | Color Slot 4 - Green (0 - 100%) |
|     12  | Color Slot 4 - Blue (0 - 100%)  |
|     13  | Color Slot 5 - Red (0 - 100%)   |
|     14  | Color Slot 5 - Green (0 - 100%) |
|     15  | Color Slot 5 - Blue (0 - 100%)  |
|     16  | Color Slot 6 - Red (0 - 100%)   |
|     17  | Color Slot 6 - Green (0 - 100%) |
|     18  | Color Slot 6 - Blue (0 - 100%)  |
|     19  | Color Slot 7 - Red (0 - 100%)   |
|     20  | Color Slot 7 - Green (0 - 100%) |
|     21  | Color Slot 7 - Blue (0 - 100%)  |

### Light Channel Table

| Channel | Function                         |
| -------:| -------------------------------- |
|      1  | Dimmer (0 - 100%)                |
|      2  | Color Slot Idx (see table below) |

| Value     | Color Slot Idx |
| ---------:| -------------- |
|   0 -  35 | Color Slot 1   |
|  36 -  71 | Color Slot 2   |
|  72 - 107 | Color Slot 3   |
| 108 - 143 | Color Slot 4   |
| 144 - 179 | Color Slot 5   |
| 180 - 215 | Color Slot 6   |
| 216 - 255 | Color Slot 7   |
