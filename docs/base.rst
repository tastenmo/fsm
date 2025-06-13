************************
Base functionality
************************

`fsm` comes with some basic functionality mostly used by other parts of the library.

.. contents:: Contents

Compressed pair
===============

Primarily designed for internal use and far from being feature complete, the
`compressed_pair` class does exactly what it promises: it tries to reduce the
size of a pair by exploiting _Empty Base Class Optimization (or _EBCO_).

This class **is not** a drop-in replacement for `std::pair`. However, it offers
enough functionalities to be a good alternative for when reducing memory usage
is more important than having some cool and probably useless feature.

Although the API is very close to that of `std::pair` (apart from the fact that
the template parameters are inferred from the constructor and therefore there is
no` entt::make_compressed_pair`), the major difference is that `first` and
`second` are functions for implementation needs:

.. code-block:: cpp
    :caption: Example of usage
    
        escad::compressed_pair pair{0, 3.};
        pair.first() = 42;


Here's the class in all its glory:

.. doxygenclass:: escad::compressed_pair
    :members:


There isn't much to describe then. It's recommended to rely on documentation and
intuition. At the end of the day, it's just a pair and nothing more.

Hashed strings
================

A hashed string is a zero overhead unique identifier. Users can use
human-readable identifiers in the codebase while using their numeric
counterparts at runtime, thus without affecting performance.

The class has an implicit `constexpr` constructor that chews a bunch of
characters. Once created, all what one can do with it is getting back the
original string through the `data` member function or converting the instance
into a number.

The good part is that a hashed string can be used wherever a constant expression
is required and no _string-to-number_ conversion will take place at runtime if
used carefully.

Example of use:

.. code-block:: cpp
    :caption: Example of usage
    
        auto load(escad::hashed_string::hash_type resource) {
        // uses the numeric representation of the resource to load and return it
        }

        auto resource = load(escad::hashed_string{"gui/background"});