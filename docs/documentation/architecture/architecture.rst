########################
Create Modifiable Tables
########################

When working with plan operations it often becomes necessary to create
new temporary tables with a certain set of columns and / or layout. To
avoid calling the base APIs every time the class ```TableBuilder```
provides very simple means to create a new modifiable Table based on a
given input specification.


The easiest way to create a new table is using the following code::

    TableBuilder::param_list list;
    list.append().set_type("FLOAT").set_name("first");
    list.append().set_type("STRING").set_name("second");
    
    AbstractTable* result = TableBuilder::build(list);

The first step is to create a list element that will include all
attributes with their respective type and name information. The method
```append()``` takes a ```param``` object as a parameter and returns a
const reference to this element which can then be set using the member
methods of the object. Each set retuns a const reference to this so
they can be simply chained as shown above.

When calling ```TableBuilder::build(list)``` the TableBuilder will
create a new table based on this information and returns it with
retain count 1.

The second possible operation is to create a new table with a specific
layout. This is done by calling the ```appendGroup``` method of the
```param_list``` class as shown in the following example::

    TableBuilder::param_list list;
    list.append().set_type("FLOAT").set_name("first");
    list.append().set_type("STRING").set_name("second");
    list.append().set_type("INTEGER").set_name("third");

    // Set the layout
    list.appendGroup(1).appendGroup(2);

    AbstractTable* result = TableBuilder::build(list);

Here, the `TableBuilder` will create a vertical table that consists
out of two tables. The first one stores one attribute, while the
second one stores two attributes. For further reference please refer
to the unit tests of the table builder class.
