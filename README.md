# POLY2TET
Program described in the jounal paper, "POLY2TET: A Computer Program for Conversion of Computational Human Phantoms from Polygonal Mesh to Tetrahedral Mesh", submitted to Journal of Radiological Protection.

## Installation
Installation can be done very simply by two lines below.
```
cmake .
make
```
## Usage
```
./POLY2TET [options] MRCP_AM.obj
  options:
  -g : generate Geant4 code
  -m : generate MCNP6 input file
  -p : generate PHITS input file
  -c : positive integer number to replace zero-region number (and not defined regions, when used as -rc)
  -mat/m : material file in MCNP6 format
  -mat/p : material file in PHITS format (will be implemented very soon)
```
## Monte Carlo dose calculation
For the Monte Carlo dose calculations, the corresponding code should be installed in the computer.
### MCNP6 (> version 6 / to use um file: > version 6.2)
Go to the folder where MCNP6 input file is generated, and then execute MCNP6 with the input file named (phantom_name)_MPNP6.i. 
```
MCNP6 n=(phantom_name)_MCNP6.i
```
### PHITS (> version 2.88)
Go to the folder where PHITS input file is generated, and then execute PHITS with the input file named (phantom_name)_PHITS.pin. 
```
phits.sh (phantom_name)_PHITS.pin
```
### Geant4 (> version 10.0)
In the folder where Geant4 source code is generated, compile the source code for Geant4 first, and execute the code with the lines below.
```
cmake .
make
./(phantom_name)
```
