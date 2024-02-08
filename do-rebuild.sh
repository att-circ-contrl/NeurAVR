#!/bin/bash
# Attention Circuits Control Laboratory - Atmel AVR firmware
# Rebuild script.
# Written by Christopher Thomas.
# Copyright (c) 2018 by Vanderbilt University. This work is licensed under
# the Creative Commons Attribution 4.0 International License.


#
# NOTE - This builds several types of library:
#
# - Baseline "neuravr" libraries (libneurXXX-mYYYY.a). These are stand-alone
# libraries for the "mYYYY" AVR architectures.
#
# - Emulation libraries (libneurXXX-mYYYY-emu.a). These are intended to be
# compiled on a development workstation, with STDIN/STDOUT pretending to be
# the USB serial link.
#
# FIXME: Arduino stubs NYI.
# The old way was to compile everything at the same time as the user's
# project using the IDE. A better way would be to compile as a library, and
# tell the IDE to link it in, while still using Arduino function calls.


# Configuration.

TOPDIR=`pwd`

CFLAGS="-I`pwd`/include -Os -c -fno-exceptions"
EMUCFLAGS="-I`pwd`/include -O2 -Wall -std=c++11 -pthread -DNEUREMU -c"

# Architectures to compile for.
read -d '' ARCHLIST <<-"Endofblock"
	m328p
	m2560
Endofblock

# Auxiliary library folders.
read -d '' AUXLIST <<-"Endofblock"
	skeleton-oo
Endofblock

# Compatibility shim library folders.
read -d '' SHIMLIST <<-"Endofblock"
	emulation
Endofblock



# Copy headers.
# These need to be in place before compiling.

rm -rf include
mkdir include

cp core/*.h include

for ARCHDIR in $ARCHLIST
do
  cp ${ARCHDIR}/neur-${ARCHDIR}*.h include
done

for AUXDIR in $AUXLIST
do
  cp ${AUXDIR}/*.h include
done

for SHIMDIR in $SHIMLIST
do
  cp ${SHIMDIR}/*.h include
done



# Clean out the existing libraries.

rm -rf lib
mkdir lib



#
# Build core firmware libraries for each of the architectures.

# Banner.
echo "== Core firmware libraries."

for ARCHDIR in $ARCHLIST
do
  # Banner.
  echo "-- $ARCHDIR"

  ARCHDEF=`cat ${ARCHDIR}/ARCHDEF`
  ARCHTARGET=`cat ${ARCHDIR}/ARCHTARGET`

  # Rebuild core.

  cd ${TOPDIR}/core

  rm -f *.o
  for CFILE in *.cpp
  do
    OFILE=`echo $CFILE|sed -e "s/cpp/o/"`

# FIXME - Diagnostics.
echo ".. core/${CFILE}"

    avr-gcc -mmcu=${ARCHTARGET} -D${ARCHDEF} $CFLAGS -o $OFILE $CFILE
  done

  # Build architecture-specific routines.

  cd ${TOPDIR}/${ARCHDIR}

  rm -f *.o
  for CFILE in *.cpp
  do
    OFILE=`echo $CFILE|sed -e "s/cpp/o/"`

# FIXME - Diagnostics.
echo ".. ${ARCHDIR}/${CFILE}"

    avr-gcc -mmcu=${ARCHTARGET} -D${ARCHDEF} $CFLAGS -o $OFILE $CFILE
  done

  # Assemble the library.

  cd ${TOPDIR}

  avr-ar -rs lib/libneur-${ARCHDIR}.a core/*.o ${ARCHDIR}/*.o

  # Clean up.
  rm -f core/*.o
  rm -f ${ARCHDIR}/*.o
done



#
# Build emulated core firmware libraries.
# The per-architecture variants have different buffer sizes.

# Banner.
echo "== Emulated core firmware libraries."

for ARCHDIR in $ARCHLIST
do
  # Banner.
  echo "-- $ARCHDIR"

  ARCHDEF=`cat ${ARCHDIR}/ARCHDEF`
  ARCHTARGET=`cat ${ARCHDIR}/ARCHTARGET`

  # Rebuild the shim.

  cd ${TOPDIR}/emulation

  rm -f *.o
  for CFILE in *.cpp
  do
    OFILE=`echo $CFILE|sed -e "s/cpp/o/"`

# FIXME - Diagnostics.
echo ".. emulation/${CFILE}"

    # NOTE - Use g++, not gcc, for the emulated version.
    g++ -D${ARCHDEF} $EMUCFLAGS -o $OFILE $CFILE
  done

  # Rebuild core.

  cd ${TOPDIR}/core

  rm -f *.o
  for CFILE in *.cpp
  do
    OFILE=`echo $CFILE|sed -e "s/cpp/o/"`

# FIXME - Diagnostics.
echo ".. core/${CFILE}"

    # NOTE - Use g++, not gcc, for the emulated version.
    g++ -D${ARCHDEF} $EMUCFLAGS -o $OFILE $CFILE
  done

  # Do not build architecture-specific code; this is what's being
  # shimmed out.

  # Assemble the library.

  cd ${TOPDIR}

  ar -rs lib/libneur-${ARCHDIR}-emu.a core/*.o emulation/*.o

  # Clean up.
  rm -f emulation/*.o
  rm -f core/*.o
done



#
# Build auxiliary libraries for each of the architectures.
# Source might not change, but compiler output may.

# Banner.
echo "== Auxiliary libraries."

for AUXDIR in $AUXLIST
do
  # Banner.
  echo "-- $AUXDIR"

  AUXNAME=`cat ${AUXDIR}/LIBNAME`

  for ARCHDIR in $ARCHLIST
  do
    # Banner.
    echo ".. $ARCHDIR"

    ARCHDEF=`cat ${ARCHDIR}/ARCHDEF`
    ARCHTARGET=`cat ${ARCHDIR}/ARCHTARGET`

    # Compile the files.

    THISDIR=${TOPDIR}/${AUXDIR}
    cd ${THISDIR}
    rm -f *.o

    for CFILE in *.cpp
    do
      OFILE=`echo $CFILE|sed -e "s/cpp/o/"`

# FIXME - Diagnostics.
echo ".. ${THISDIR}/${CFILE}"

      avr-gcc -mmcu=${ARCHTARGET} -D${ARCHDEF} $CFLAGS -o $OFILE $CFILE
    done

    # Assemble the library.

    cd ${TOPDIR}

    avr-ar -rs lib/lib${AUXNAME}-${ARCHDIR}.a ${THISDIR}/*.o

    # Clean up.
    rm -f ${THISDIR}/*.o
  done
done



#
# Build emulated auxiliary libraries for each of the architectures.
# NOTE - Per-architecture output really should be the same for all of these,
# but build them separately just in case that doesn't stay the case.

# Banner.
echo "== Emulated auxiliary libraries."

for AUXDIR in $AUXLIST
do
  # Banner.
  echo "-- $AUXDIR"

  AUXNAME=`cat ${AUXDIR}/LIBNAME`

  for ARCHDIR in $ARCHLIST
  do
    # Banner.
    echo ".. $ARCHDIR"

    ARCHDEF=`cat ${ARCHDIR}/ARCHDEF`
    ARCHTARGET=`cat ${ARCHDIR}/ARCHTARGET`

    # Compile the files.

    THISDIR=${TOPDIR}/${AUXDIR}
    cd ${THISDIR}
    rm -f *.o

    for CFILE in *.cpp
    do
      OFILE=`echo $CFILE|sed -e "s/cpp/o/"`

# FIXME - Diagnostics.
echo ".. ${THISDIR}/${CFILE}"

      gcc -D${ARCHDEF} $EMUCFLAGS -o $OFILE $CFILE
    done

    # Assemble the library.

    cd ${TOPDIR}

    ar -rs lib/lib${AUXNAME}-${ARCHDIR}-emu.a ${THISDIR}/*.o

    # Clean up.
    rm -f ${THISDIR}/*.o
  done
done



#
# Ending banner.

echo "== Done."


# This is the end of the file.
