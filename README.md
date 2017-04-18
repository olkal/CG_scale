# CG_scale
Arduino based CG scale for F3X gliders (and other model airplanes)
The scale can be used for most modern F3F/F3B gliders with slim fuselages. It was designed to be as compact and stiff as possible, so there is not too much clearance for fat fuselages and F3B hooks. The load sensor cover can be removed for some extra vertical clearance for hook if needed. 

3D printed parts:
The parts should be printed with the same orientation as in the STL files. The load sensor cover can be turned upside down if you want to print without support. PLA material with generous infill is recommended. Penetrating holes are blinded for better print quality (continuous perimeter), these have to be drilled trough after printing.

Assembly, mechanical parts:
Install M3 nuts in the frame. The load sensors contact surfaces must be flat and true, and may need some work with a small file. Check overall alignment, needless to say everything must be straight and true for the scale to be accurate. Use a file on the plastic parts if any adjustment is required. IMPORTANT: The 4 wing pads are angled for wing dihedral and must be installed with correct orientation. The hinging for the wing pads must be loose to avoid any binding and to give some allowance for different wing dihedrals. Use a 2mm drillbit on the hole in the wing pads if they are tight. Attach soft sticker pads (furniture type) on wing pads and under the frame.

Calibration:
The two load sensors must be calibrated with a known weight. I'm using a 500gr calibration weight for this myself.
Measure the average projected distance between the front and rear wing pad hinge points
Measure the average projected distance between the front wing pad hinge points and the leading edge stopper pins
The values must be written to EEPROM by un-commenting the code in setup.  After uploading, re-comment and upload again

Use:
The model is measured with a slight "nose down" angle. This is intentional and is not adjustable, but it is possible to change by modifying the front or the rear support height. If properly built, the scale is as accurate as you can place the model on it. Try to relocate the model a few times to verify that the measured CG value is repeatable.

To do:
Electronics
BOM
  
