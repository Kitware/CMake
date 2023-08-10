#include <hdf5.h>

int main(int argc, const char* argv[])
{
  hid_t fid;
  if (argc != 2) {
    return 1;
  }
  fid = H5Fcreate(argv[1], H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  H5Fclose(fid);
  return 0;
}
