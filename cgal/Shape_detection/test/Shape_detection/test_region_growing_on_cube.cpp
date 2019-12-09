// STL includes.
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iterator>
#include <cassert>

// CGAL includes.
#include <CGAL/assertions.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Iterator_range.h>
#include <CGAL/HalfedgeDS_vector.h>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>

#include <CGAL/Shape_detection/Region_growing/Region_growing.h>
#include <CGAL/Shape_detection/Region_growing/Region_growing_on_polygon_mesh.h>

namespace SD = CGAL::Shape_detection;

template<class Kernel>
bool test_region_growing_on_cube(int argc, char *argv[]) {

  using FT = typename Kernel::FT;

  using Polyhedron = CGAL::Polyhedron_3<Kernel, CGAL::Polyhedron_items_3, CGAL::HalfedgeDS_vector>;
  using Face_range = typename CGAL::Iterator_range<typename boost::graph_traits<Polyhedron>::face_iterator>;

  using Neighbor_query = SD::Polygon_mesh::One_ring_neighbor_query<Polyhedron, Face_range>;
  using Region_type    = SD::Polygon_mesh::Least_squares_plane_fit_region<Kernel, Polyhedron, Face_range>;
  using Region_growing = SD::Region_growing<Face_range, Neighbor_query, Region_type>;

  // Default parameter values for the data file cube.off.
  const FT          distance_threshold = FT(1) / FT(10);
  const FT          angle_threshold    = FT(25);
  const std::size_t min_region_size    = 1;

  // Load data.
  std::ifstream in(argc > 1 ? argv[1] : "data/cube.off");
  CGAL::set_ascii_mode(in);

  Polyhedron polyhedron;
  in >> polyhedron;
    
  in.close();
  const Face_range face_range = faces(polyhedron);

  assert(face_range.size() == 6);
  if (face_range.size() != 6) 
    return false;

  // Create parameter classes.
  Neighbor_query neighbor_query(polyhedron);
  
  Region_type region_type(
    polyhedron, 
    distance_threshold, angle_threshold, min_region_size);

  // Run region growing.
  Region_growing region_growing(
    face_range, neighbor_query, region_type);
  
  std::vector< std::vector<std::size_t> > regions;
  region_growing.detect(std::back_inserter(regions));

  // Test data.
  assert(regions.size() == 6);
  
  if (regions.size() != 6) 
    return false;

  for (const auto& region : regions)
    if (!region_type.is_valid_region(region)) 
      return false;

  std::vector<std::size_t> unassigned_faces;
  region_growing.unassigned_items(std::back_inserter(unassigned_faces));

  assert(unassigned_faces.size() == 0);

  if (unassigned_faces.size() != 0) 
    return false;

  return true;
}

int main(int argc, char *argv[]) {

  // ------>

  bool cartesian_double_test_success = true;
  if (!test_region_growing_on_cube< CGAL::Simple_cartesian<double> >(argc, argv)) 
    cartesian_double_test_success = false;
    
  std::cout << "cartesian_double_test_success: " << cartesian_double_test_success << std::endl;
  assert(cartesian_double_test_success);

  // ------>

  bool exact_inexact_test_success = true;
  if (!test_region_growing_on_cube<CGAL::Exact_predicates_inexact_constructions_kernel>(argc, argv)) 
    exact_inexact_test_success = false;
    
  std::cout << "exact_inexact_test_success: " << exact_inexact_test_success << std::endl;
  assert(exact_inexact_test_success);

  // ------>

  bool exact_exact_test_success = true;
  if (!test_region_growing_on_cube<CGAL::Exact_predicates_exact_constructions_kernel>(argc, argv)) 
    exact_exact_test_success = false;
    
  std::cout << "exact_exact_test_success: " << exact_exact_test_success << std::endl;
  assert(exact_exact_test_success);

  const bool success = cartesian_double_test_success && exact_inexact_test_success && exact_exact_test_success;
  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}
