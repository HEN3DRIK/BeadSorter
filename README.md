 - 👋 Hi, I’m @HEN3DRIK
 - 👀 I’m interested in 3DPrinting and Electroforming/Electroplating
 - 📫 How to reach me 
   -  Youtube https://www.youtube.com/c/v0g3l
   - Twitter https://twitter.com/v0g3l

# BeadSorter
Nice to have you here. In this repository I keep the files that are necessary for the BeadSorter. Also there will be documentation about the needed electronics and software. I will update this repo continuously.

# Video
[![perlerbead sorting machine](https://img.youtube.com/vi/CX-w85ZC5AQ/0.jpg)](https://www.youtube.com/watch?v=CX-w85ZC5AQ)

# References
These are Projects are necessary for this beadsorter to work.

Original Project by Linus Barth:
[BeadSort: Machine for sorting perler beads by LinusB - Thingiverse](https://www.thingiverse.com/thing:2598302)

Modified to skip aligner phase by RichA777
[Bead Sorter modified to skip aligner phase by RichA777 - Thingiverse](https://www.thingiverse.com/thing:4507571)
# Parts List
You'll need electronic parts for this bead sorter. This is alist of what i used in this project. I belive some components can be substituted.

- 1x Miniature-DC-Gear Motor 15RPM
- 2x LM2596S DC-DC
- 1x A4988 Stepper Driver Module
- 1x Nema 17 Stepper Bipolar 1A 16Ncm 42x20mm 4-Wires
- 1x L298N Motor Drive Controller Board Module
- 1x Adafruit-Sensor TCS34725 RGB
- 1x Arduino Nano V3.0 Atmega328 CH340
- 1x SG90 Micro Servo Motor 9G
- 1x 100µF Capacitor
- 1x Resistor matching LED
- 1x White LED
- 1x Photo Resistor 5mm
- 1x Push Button
- Wires
- PTFE Spray

# Electronics Schema
I know, this isn't nice looking.
![alt text](https://github.com/HEN3DRIK/BeadSorter/blob/main/beadsorter_schema.png?raw=true)

# Printing Guide
## Printing Linus Barth Parts
Print everything but the following Parts:
- 0-hopper-seq-plate.stl
- 0-hopper.stl
- 1-sequentializer-base.stl
- 1-sequentializer-ilc-corner.stl
- 1-sequentializer-tube-inset.stl
- 2-*.stl
- 3-analyzer-entry-tube.stl
- 4-dispatcher-tube-holder.stl
- 4-dispatcher-servo-holder.stl
- 4-dispatcher-base.stl
- 5-outputs-base.stl
- 5-outputs-slide-1.stl
- 5-outputs-slide-2.stl
- 5-outputs-slide-3.stl
- rpi-holder.stl

## Printing RichA777 parts
Print the following parts
- 0-hopper-seq-plate-4_holes_modified
- Sequencer_base_modified
- 0_Hopper_Modified_Cleaned_up

## Printing my parts
Print:
- 4x 1-sequentializer-ilc-corner_for_higher_motor.stl
- 0-ring_container.stl
- 4-Stepper_base.stl
- 4-Stepper_shaft.stl
- 4-Stepper_shaft_ring.stl
- 4-Servo_base2.stl
- 4-Stepper_shaft_ring.stl
- 3-analyzer-entry-tube_modified.stl
- 3-Sequencer_tube_modified_tighter_Higher_Motor.stl
- 5-outputs-slide-1.stl
- 5-outputs-slide-2.stl
- 5-outputs-slide-3.stl
- 5-holder_typeA.stl
- 5-holder_typeB.stl
- 5-Plate.stl
- 6-Box_bottom.stl
- 6-Box_lid.stl
- 6-Box_top.stl
