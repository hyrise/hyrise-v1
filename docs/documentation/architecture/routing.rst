#######
Routing
#######

The `net` library allows other libraries to register handlers for URL
prefixes to specific handlers.

.. doxygenclass:: hyrise::net::Router
   :members: registerRoute, dispatch
   
