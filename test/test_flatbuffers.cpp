//
// Created by slovak on 12/17/20.
//

#include <iostream>

#include <thread>
#include <fstream>
#include <chrono>

//#include <nlohmann/json.hpp>

#include "test_generated.h"

#include "flatbuffers/minireflect.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"

#include "Json2Eigen.hpp"


using namespace std::chrono_literals;

namespace flatbuffers {

inline std::string FlatBufferToStringFancy(const uint8_t *buffer,
                                      const TypeTable *type_table,
                                      bool multi_line = false,
                                      bool vector_delimited = true) {
  ToStringVisitor tostring_visitor("\n" , true, " ",
                                   vector_delimited);
  IterateFlatBuffer(buffer, type_table, &tostring_visitor);
  return tostring_visitor.s;
}

}


template <typename T>
auto as_eigen_matrix_dynamic(T* data, int rows, int cols) {
  Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic,Eigen::Dynamic, Eigen::RowMajor>> map(data, rows, cols);
  return map;
}

int load_json(void) {
  std::string schemafile = "struct Vec3 { v:[float32:3];}struct Joy { k_j_omega: float32; k_j_x_dot: float32; v3:Vec3;}table Conf { joy:Joy; some2:[float32];}root_type Conf;";
  std::string jsonfile;
  bool ok = /*flatbuffers::LoadFile("/home/slovak/pibot/config/test.fbs", false, &schemafile) &&*/
      flatbuffers::LoadFile("/home/slovak/pibot/config/conf_test.json", false, &jsonfile);
  if (!ok) {
    printf("couldn't load files!\n");
    return 1;
  }

  // parse schema first, so we can use it to parse the data after
  flatbuffers::Parser parser;
  ok = parser.Parse(schemafile.c_str()) && parser.Parse(jsonfile.c_str());
  assert(ok);

  // here, parser.builder_ contains a binary buffer that is the parsed data.

  // to ensure it is correct, we now generate text back from the binary,
  // and compare the two:
  std::string jsongen;
  if (!GenerateText(parser, parser.builder_.GetBufferPointer(), &jsongen)) {
    printf("Couldn't serialize parsed data to JSON!\n");
    return 1;
  }

//  printf("%s----------------\n%s", jsongen.c_str(), jsonfile.c_str());


//  std::cout << schemafile << std::endl;

}


//int main(int argc, char *argv[]) {
//
//  Eigen::Matrix<float, 4,3> mat43;
//
//  mat43
//  << 1,2,3, \
//   1,2,3, \
//   1,2,3, \
//   1,2,3;
//
//   std::cout << mat43 << std::endl<< std::endl;
//
//  FbConfT fb_conf;
//
//  fb_conf.mat = std::make_unique<FbEigenMatT>();
//
//  fb_conf.mat->cols = mat43.cols();
//  fb_conf.mat->rows = mat43.rows();
//  fb_conf.mat->dtype = "f4";
//  fb_conf.mat->order = "F";
//
//
//  for (std::size_t row = 0; row < mat43.rows(); row++) {
//    for (std::size_t col = 0; col < mat43.cols(); col++) {
//      fb_conf.mat->data.push_back(mat43(row, col));
//    }
//  }
//
//  flatbuffers::FlatBufferBuilder fbb;
//  fbb.Finish(FbConf::Pack(fbb, &fb_conf));
//
//  auto config_ptr = UnPackFbConf(fbb.GetBufferPointer());
//
//  auto map = as_eigen_matrix_dynamic(config_ptr->mat->data.data(), config_ptr->mat->rows, config_ptr->mat->cols);
//
//  std::cout << map << std::endl;
//  std::cout << flatbuffers::FlatBufferToStringFancy(fbb.GetBufferPointer(), FbConfTypeTable(), true, false) << std::endl;
//
//  return 0;
//}

int main(int argc, char *argv[]) {

  return 0;
}