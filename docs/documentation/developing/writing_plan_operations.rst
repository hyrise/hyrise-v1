#######################
Writing plan operations
#######################


Json Parsing
============

Every object of type T that can be created through Json should expose a method::

    T* T::parse(Json::Value&);

where it constructs itself.
