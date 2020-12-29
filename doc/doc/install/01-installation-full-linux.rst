:orphan:

.. _sec-installation-full-linux:

#####################################
Installing Tubex on Linux for C++ use
#####################################


Requirements and dependencies (IBEX)
------------------------------------

Tubex uses several features of the `IBEX library <http://www.ibex-lib.org/doc/install.html>`_ that you have to install first. The last version of IBEX is available on `the official development repository <https://github.com/ibex-team/ibex-lib>`_:

.. code-block:: bash

  # Requirements to compile IBEX and Tubex
  sudo apt-get install -y g++ gcc flex bison cmake git libeigen3-dev
  
  # Download IBEX sources from Github
  git clone -b develop https://github.com/ibex-team/ibex-lib.git
  
  # Configure IBEX before installation
  cd ibex-lib
  mkdir build && cd build
  cmake ..
  
  # Building + installing
  make
  sudo make install
  cd ..

For further CMake options, please refer to the IBEX documentation. 

.. admonition:: Debug/development mode
  
  Note that the :code:`-DCMAKE_BUILD_TYPE=Debug` option will slightly slow down your computations, but display useful error messages in case of failure conditions such as access violations. **It is highly recommended** for your developments.

  To use it:

  .. code-block:: bash
  
    cmake -DCMAKE_BUILD_TYPE=Debug .. 


Building the Tubex library
--------------------------

The last sources are available on `the official Tubex development repository <https://github.com/SimonRohou/tubex-lib>`_. You can download the files and compile the sources with:

.. code-block:: bash

  git clone https://github.com/SimonRohou/tubex-lib   # download the sources from GitHub
  cd tubex-lib                                        # move to the Tubex directory
  git submodule init ; git submodule update           # get pybind11 submodule
  mkdir build ; cd build ; cmake .. ; make            # build the sources
  sudo make install                                   # install the library
  cd ..                                               # back to the root of Tubex


Compiling the examples
----------------------

To compile one specific example, use CMake in the example directory.
For instance:

.. code-block:: bash
  
  cd examples/basics/ex_01_arithmetic                 # moving to the example directory
  mkdir build -p ; cd build ; cmake .. ; make         # cmake compilation
  ./tubex_basics_01                                   # running example

Do not forget to launch the VIBes viewer before running your program.


(for experts) Additional installation options
---------------------------------------------

.. _sec-installation-full-linux-cmake:

.. rst-class:: fit-page

  CMake supports the following options:

  ======================  ======================================================================================
  Option                  Description
  ======================  ======================================================================================
  CMAKE_INSTALL_PREFIX    | By default, the library will be installed in system files (:file:`/usr/local/` under Linux).
                            Use ``CMAKE_INSTALL_PREFIX`` to specify another path.
                          | Example:

                          .. code-block:: bash

                            cmake -DCMAKE_INSTALL_PREFIX=$HOME/tubex-lib/build_install ..
                          
                          .. warning::
                          
                            The full path of the folder must not contain white space or weird characters like ``'"\()`*[]``.

  CMAKE_BUILD_TYPE        | Set the build mode either to ``Release`` or ``Debug``.
                          | Default value is ``Debug``. Example:

                          .. code-block:: bash

                            cmake -DCMAKE_BUILD_TYPE=Release ..
                
                          The :code:`-DCMAKE_BUILD_TYPE=Debug` option is enabled by default. As for IBEX, it will slightly
                          slow down your computations, but display useful error messages in case of failure conditions such
                          as access violations. It is highly recommended for your developments. You can otherwise use the
                          :code:`-DCMAKE_BUILD_TYPE=Release` option. Note also that O3 optimizations are always activated.
                          
                          Once Tubex has been compiled with this option, you should also compile your executable
                          in debug mode.

  CMAKE_PREFIX_PATH       | If IBEX has been installed in a local folder, say :file:`~/ibex-lib/build_install`, you need
                            to indicate this path using the ``CMAKE_PREFIX_PATH`` option.
                          | Example:

                          .. code-block:: bash

                            cmake -DCMAKE_PREFIX_PATH=$HOME/ibex-lib/build_install ..
  ======================  ======================================================================================


.. admonition:: Custom install directory of IBEX and Tubex
  
  A convenient way to refer to custom install directories for IBEX and/or Tubex is to export the ``CMAKE_PREFIX_PATH`` environment variable. For instance:

  .. code-block:: bash

    export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:$HOME/ibex-lib/build_install
    export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:$HOME/tubex-lib/build_install