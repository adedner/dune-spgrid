dune_define_gridtype(GRID_CONFIG_H_BOTTOM
                     GRIDTYPE SPGRID
                     ASSERTION "GRIDDIM == WORLDDIM"
                     DUNETYPE "Dune::SPGrid< double, dimgrid >"
                     HEADERS "dune/grid/spgrid.hh" "dune/grid/spgrid/dgfparser.hh")
dune_define_gridtype(GRID_CONFIG_H_BOTTOM
                     GRIDTYPE SPGRID_SERIAL
                     ASSERTION "GRIDDIM == WORLDDIM"
                     DUNETYPE "Dune::SPGrid< double, dimgrid, SPIsotropicRefinement, No_Comm >"
                     HEADERS "dune/grid/spgrid.hh" "dune/grid/spgrid/dgfparser.hh")

dune_define_gridtype(GRID_CONFIG_H_BOTTOM
                     GRIDTYPE SPGRID_ISOTROPIC
                     ASSERTION "GRIDDIM == WORLDDIM"
                     DUNETYPE "Dune::SPGrid< double, dimgrid, SPIsotropicRefinement >"
                     HEADERS "dune/grid/spgrid.hh" "dune/grid/spgrid/dgfparser.hh")
dune_define_gridtype(GRID_CONFIG_H_BOTTOM
                     GRIDTYPE SPGRID_ANISOTROPIC
                     ASSERTION "GRIDDIM == WORLDDIM"
                     DUNETYPE "Dune::SPGrid< double, dimgrid, SPAnisotropicRefinement >"
                     HEADERS "dune/grid/spgrid.hh" "dune/grid/spgrid/dgfparser.hh")
dune_define_gridtype(GRID_CONFIG_H_BOTTOM
                     GRIDTYPE SPGRID_BISECTION
                     ASSERTION "GRIDDIM == WORLDDIM"
                     DUNETYPE "Dune::SPGrid< double, dimgrid, SPBisectionRefinement >"
                     HEADERS "dune/grid/spgrid.hh" "dune/grid/spgrid/dgfparser.hh")

dune_define_gridtype(GRID_CONFIG_H_BOTTOM
                     GRIDTYPE SPGRID_ISOTROPIC_SERIAL
                     ASSERTION "GRIDDIM == WORLDDIM"
                     DUNETYPE "Dune::SPGrid< double, dimgrid, SPIsotropicRefinement, No_Comm >"
                     HEADERS "dune/grid/spgrid.hh" "dune/grid/spgrid/dgfparser.hh")
dune_define_gridtype(GRID_CONFIG_H_BOTTOM
                     GRIDTYPE SPGRID_ANISOTROPIC_SERIAL
                     ASSERTION "GRIDDIM == WORLDDIM"
                     DUNETYPE "Dune::SPGrid< double, dimgrid, SPAnisotropicRefinement, No_Comm >"
                     HEADERS "dune/grid/spgrid.hh" "dune/grid/spgrid/dgfparser.hh")
dune_define_gridtype(GRID_CONFIG_H_BOTTOM
                     GRIDTYPE SPGRID_BISECTION_SERIAL
                     ASSERTION "GRIDDIM == WORLDDIM"
                     DUNETYPE "Dune::SPGrid< double, dimgrid, SPBisectionRefinement, No_Comm >"
                     HEADERS "dune/grid/spgrid.hh" "dune/grid/spgrid/dgfparser.hh")
dune_define_gridtype(GRID_CONFIG_H_BOTTOM
                     GRIDTYPE SPGRID_COUNT_FLOPS
                     ASSERTION "GRIDDIM == WORLDDIM"
                     DUNETYPE "Dune::SPGrid< Dune::SPGridCountFlopsDoubleType, dimgrid >"
                     HEADERS "dune/grid/spgrid.hh" "dune/grid/spgrid/dgfparser.hh")
