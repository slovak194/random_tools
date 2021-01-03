#include <remote_config/Server.h>

int main(int argc, char **argv) {

  remote_config::Server conf("/home/slovak/remote-config/config/conf_test.yaml");

  for (int i = 0; i < 1000; i++) {
    std::this_thread::sleep_for(1s);

    std::cout << "use config: " << conf("/test/vector/data/0").get<float>() << std::endl;

  }

  return 0;
}
