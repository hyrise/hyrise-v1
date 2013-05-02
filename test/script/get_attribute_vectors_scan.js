function hyrise_run_op(input) {

	var avs =input[0].getAttributeVectors(0);

	var av0 = avs[0].attribute_vector;
	var of0 = avs[0].attribute_offset;
	var size = av0.size();

	var pos = []
	for(var i=0; i < size; ++i) {
		var tmp = av0.get(of0, i);
		if ( tmp < 5 && tmp % 2 == 0)
			pos.push(i);
	}

	return createPointerCalculator(input[0], pos);
}