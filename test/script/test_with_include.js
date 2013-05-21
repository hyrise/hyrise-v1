include("simple")

function hyrise_run_op(input) {
	var x = copyStructureModifiable(input[0]);
	test();
	return x;
}