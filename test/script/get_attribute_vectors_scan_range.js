function hyrise_run_op(input) {

	var avs =input[0].getAttributeVectors(0);

	var av0 = avs[0].attribute_vector;
	var of0 = avs[0].attribute_offset;
	var size = av0.size();

	log(av0.size());

	var i =0;
	var batch = 2;

	var pos = [];

	while(i < size ) {
		var upper = i + batch;

		if (upper > size) {
			upper = size;
			batch = upper - i;
		}

		var d = av0.getRange(of0, i, upper);
		for(var j=0; j < batch; ++j) {
			var tmp = d[j];
			if ( tmp < 5 && tmp % 2 == 0)
				pos.push(i+j);
			
		}

		i = upper;
	}

	return createPointerCalculator(input[0], pos);
}