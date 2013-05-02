function hyrise_run_op(input) {
	var x = copyStructureModifiable(input[0]);
	x.resize(1);
	x.setValueInt(0,0, 1);
	x.setValueFloat(1,0, 2.0);
	x.setValueString(2,0, "Hallo");
	x.setValueInt(3,0, 1);
	x.setValueInt(4,0, 2);
	x.setValueInt(5,0, 3);
	x.setValueInt(6,0, 4);
	x.setValueInt(7,0, 5);
	x.setValueInt(8,0, 6);
	x.setValueInt(9,0, 7);
	return x;
}