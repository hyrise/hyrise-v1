(function ($) {

	var SCROLL_OFFSET = 60;
	var SCROLL_OFFSET_COLLAPSED = 40;
	var SCROLL_DURATION = 100;

	$(document).ready(function () {


		var isNarrow = function() {
			return $('.navbar-wrapper').css('display') === 'none';
		};

		// $(window).on('hashchange', function(event) {

		// 	var params = (event.originalEvent.newURL||'').split('#');
		// 	var page = params[0];
		// 	var anchor = params[1];

		// 	// if link on same page, animate
		// 	if (window.location.href.split('#')[0] === page) {			
		// 		var target = $('#' + anchor);

		// 		$('body, html').animate({
		// 		    scrollTop: (target[0] && target.offset().top || 0) - (isNarrow() ? SCROLL_OFFSET_COLLAPSED : SCROLL_OFFSET)
		// 		}, SCROLL_DURATION);
		// 	}

		// });

		var $body = $('body');
		var $menu = $('.hyrisesidebar');
		var $list = $menu.find('ul.current:last');

		/**
		 * Add special handling for small screen sizes: Add a expand/collapse property to the sidebar so it can overlay the main container
		 * Only use this functionality if the screen size is lower than 980px (bootstrap trigger size for responsive layout)
		 * @return {[type]} [description]
		 */
		var initSidebarHandling = function() {
			var handle = $menu.find('.resize-handle');

			handle.on('click', function() {
				$menu.toggleClass('sidebar-collapsed');
			})
		};

		/**
		 * set the height of the sidebar according to the height of the window
		 * @return {[type]} [description]
		 */
		var handleWindowResize = function() {

			$menu.affix('checkPosition');

			var top = $('.navbar').height();
			var scroll = $('.inner-wrapper .upper').height()// - $(window).scrollTop();

			var height = Math.min(scroll, $(window).height()) - top - (isNarrow() ? 0 : 25);

			$menu.height(height);
			// $menu.affix(();
		};

		var handleLocalTocClick = function(event) {
			// use custom scrollTo function instead of default implementation

			try {
				var target = event.target;
				target = $($(target).attr('href'));

				// only prevent default action if target was found
				event.preventDefault();

				$('body, html').animate({
				    scrollTop: (target[0] && target.offset().top || 0) - (isNarrow() ? SCROLL_OFFSET_COLLAPSED : SCROLL_OFFSET)
				}, SCROLL_DURATION);				
			} catch(e) {
				// couldn't find target
			}
		};

		var initFooter = function() {
			$menu.affix({
				offset: {
					bottom	: function() {
						return $('.footer_toc').outerHeight(true) + $('.footer').outerHeight(true) + 45;	
					}
				}
			})

		};

		var initScrollSpy = function() {
			$menu.find('ul li:not(:has(ul))').addClass('leaf');

			$list.find('a').click(handleLocalTocClick);

			if (location.pathname.match(/\/documentation/)) {
				$body[0].setAttribute('data-spy', 'scroll');
				$body[0].setAttribute('data-target', '.hyrisesidebar');
				
				// init scroll spy
				var spy = $body.scrollspy({
					offset: (isNarrow() ? SCROLL_OFFSET_COLLAPSED : SCROLL_OFFSET) + 1
				});

				$(window).resize(function() {
					$('body').data('bs.scrollspy').options.offset = (isNarrow() ? SCROLL_OFFSET_COLLAPSED : SCROLL_OFFSET) + 1;
					$('body').data('bs.scrollspy').process();
				})
			}
		};

		var doit;
		$(window).resize(function(){
		  clearTimeout(doit);
		  doit = setTimeout(handleWindowResize, 100);
		});

		initFooter();
		initSidebarHandling();
		initScrollSpy();
		handleWindowResize();

	});

}($jqTheme || window.jQuery));