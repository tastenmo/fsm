.. sphinx-escad-docs documentation

The ESCAD fsm library
======================

This modern C++ library is a header-only library.

It contains:

* some common types and classes used by the other parts of the library
* a dense_map container in addition to the stl containers
* a signal/slot implementation
* a state machine implementation
* basic logging functionality
* intrusive reflections
  
.. note::

    This library is in a beta phase.

    It is not bug free and documentation is also missing some minor stuff.
    You can help us to make it better by reporting bugs or by providing code/docs
    changes via a PR.
    
    The code is available on github: `tastenmo/fsm <https://github.com/tastenmo/fsm.git>`__
    
    A mirror is available on csvn: `CPP/fsm <http://csvn:3000/CPP/fsm.git>`__



.. if-builder:: escaddocs

    .. toctree::

       installation
       base


.. if-builder:: HTML

    .. toctree::
       :caption: Content
       :maxdepth: 3

       installation
       base
