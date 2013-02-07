**********************
HEP 0001: A new loader
**********************

HYRISE needs a new loader. It should fulfill the following
requirements

* Simple api (move away from load_with_string_header(...) and adding
  a different method for every single way we have when loading
  tables.
* Adding new loading parameters/options should not mean having to make all
  code paths aware, i.e. build one general loading logic.
* Completely move to libcsv for all inputs that are CSV-like
  i.e. current .data format with separator "|"  and .csv
* Allow specification of allocation strategies
* Reach a point where the Storage Manager does not have to know more
  than the parameters for loading stored in a single object/struct and
  can just call Loader::load(params); to retrieve a table.

First stab
==========

The general API to load a table should be concentrated in one function
call::

    AbstractTable* Loader::load(Loader::params params);

Where the parameters are an object that basically allows us to plug in 
options without having to change the function signature. Their
definition should be an enhanced version of the current method shown
in Loader.h::

    #define  param_member(<TYPE>, <NAME>)...

    struct params {
       param_member(AbstractInput*, Input);
       
       params(): Input(NULL)
       {}

       ...
    };

which then allows us to do::

    AbstractInput* input = some_input_getter();
    Loader::load(Loader::params().setInput(input).setOtherProp(1));

And then within the definition of `Loader::load`, we can do the
following::

    void Loader::load(params p)
       AbstractInput* input = p.getInput();
       /* do more stuff... */

The idea is to encapsulate all options in a struct, so we can pass
around a struct with all the necessary information to minimize
necessary code changes when a new parameter is added.

Defining different loading formats
==================================

Loading distinguishes between the header for metadata generation,
input data loading for "inserting" actual data and general options
such as allocation strategies.

Here is a pseudo implementation of how the loader is supposed to
work::

    AbstractTable* Loader::load(params p)
        Metadata m = p.getHeader().getMetadata();
        TableFactory tf = p.getTableFactory();
        AbstractTable new_table = tf.generate(m);
        p.getInput().load(p, new_table);
        /* do some more stuff... wrap in Store etc pp */
        return new_table;
    
       
We define the following abstract classes::

    namespace loaders
        class AbstractInput...
        class AbstractMetadata...
    

Then we can provide specific implementations for different Input and
Metadata sources. 

For example, we want to be able to load data from CSV files. A pseudo
implementation of such an endeavour could look like this::

    class CSVDataInput : public AbstractInput
        
        CSVInput(string filename, has_header=false) :
        _filename(filename), _has_header(has_header) {}

        AbstractTable* load(Loader::params p, AbstractTable* target);
            /* open file */
            if has_header == True
                /* skip first four lines */
           
            /* use libcsv to build column loaders and load data into
            target */
 
Similarly, we need to define a metadata yielding class to understand
the structure of a table.::

    class CSVMetadata : public AbstractMetadata

         CSVMetadata(string filename) : _filename(filename) {}

         Metadata getMetadata()
             /* read and process first four lines */
             return metadata;


Thus allowing us to load a table as follows::

    Loader::load(Loader::params()
                 .setInput(CSVInput("tables/customer.csv").hasHeader())
                 .setHeader(CSVMetadata("tables/customer.csv")));


Or::

    Loader::load(Loader::params()
                 .setInput(EmptyInput())
                 .setHeader(CSVMetadata("tables/customer.csv")));
             
As can be seen above, in order to avoid long parameter lists, we shall
implement the `Named parameter idiom`_ for all implementations.

.. _`Named parameter idiom` : http://www.parashift.com/c++-faq-lite/ctors.html#faq-10.20
