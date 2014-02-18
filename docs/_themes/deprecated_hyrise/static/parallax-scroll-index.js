$(document).ready(function() {

	var $window = $(window);
	var $firstBG = $('#intro');
	
	var repositionNav = function() {
		var scrollY = window.scrollY;
		if (scrollY >= 350) {
			$('#menu').css({
				top:0,
				position: 'fixed',
				margin: 'auto'
			});
		} else {
			$('#menu').css({
				position: 'relative',
				margin: '-20px -20px -30px -20px',
				top: '0px'
			});
		}
	};

	var newPos = function(x, pos, adjuster, inertia) {
		// var windowHeight = $window.height();
		return x + "% " + (-((-100+pos)) * inertia)  + "px";
	};

	var move = function() {
		var pos = $window.scrollTop();
		$firstBG.css({'backgroundPosition': newPos(70, pos, 800, 0.5)});
		// position the nav bar
		repositionNav();
	};

	$window.resize(function(){
		move();
	});

	$window.bind('scroll', function(){
		move();
	});

	move();

});