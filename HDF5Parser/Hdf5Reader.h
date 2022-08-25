/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://support.hdfgroup.org/ftp/HDF5/releases.  *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 //
 //      This example reads hyperslab from the SDS.h5 file into
 //      two-dimensional plane of a three-dimensional array.  Various
 //      information about the dataset in the SDS.h5 file is obtained.
 //

#ifdef OLD_HEADER_FILENAME
#include <iostream.h>
#else
#include <iostream>
#endif
using std::cout;
using std::endl;
#include <string>
#include "H5Cpp.h"
using namespace H5;

#define OBJ_NAME_LEN 260

int Read(void)
{

	const char* read_file("SDS.h5");
	const char* read_dataset("IntArray");

	const int    NX_SUB = 3;    // hyperslab dimensions
	const int    NY_SUB = 4;
	const int    _NX = 7;        // output buffer dimensions
	const int    _NY = 7;
	const int    _NZ = 3;
	const int    RANK_OUT = 3;
    /*
     * Output buffer initialization.
     */
    int i, j, k;
    int         data_out[_NX][_NY][_NZ]; /* output buffer */
    for (j = 0; j < _NX; j++)
    {
        for (i = 0; i < _NY; i++)
        {
            for (k = 0; k < _NZ; k++)
                data_out[j][i][k] = 0;
        }
    }
    /*
     * Try block to detect exceptions raised by any of the calls inside it
     */
    try
    {
        /*
         * Turn off the auto-printing when failure occurs so that we can
         * handle the errors appropriately
         */
        Exception::dontPrint();
        /*
         * Open the specified file and the specified dataset in the file.
         */
        H5File file(read_file, H5F_ACC_RDONLY);
        DataSet dataset = file.openDataSet(read_dataset);
        /*
         * Get the class of the datatype that is used by the dataset.
         */
        H5T_class_t type_class = dataset.getTypeClass();
        /*
         * Get class of datatype and print message if it's an integer.
         */
        if (type_class == H5T_INTEGER)
        {
            cout << "Data set has INTEGER type" << endl;
            /*
         * Get the integer datatype
             */
            IntType intype = dataset.getIntType();
            /*
             * Get order of datatype and print message if it's a little endian.
             */
            H5std_string order_string;
            H5T_order_t order = intype.getOrder(order_string);
            cout << order_string << endl;
            /*
             * Get size of the data element stored in file and print it.
             */
            size_t size = intype.getSize();
            cout << "Data size is " << size << endl;
        }
        /*
         * Get dataspace of the dataset.
         */
        DataSpace dataspace = dataset.getSpace();
        /*
         * Get the number of dimensions in the dataspace.
         */
        int rank = dataspace.getSimpleExtentNdims();
        /*
         * Get the dimension size of each dimension in the dataspace and
         * display them.
         */
        hsize_t dims_out[2];
        int ndims = dataspace.getSimpleExtentDims(dims_out, NULL);
        cout << "rank " << rank << ", dimensions " <<
            (unsigned long)(dims_out[0]) << " x " <<
            (unsigned long)(dims_out[1]) << endl;
        /*
         * Define hyperslab in the dataset; implicitly giving strike and
         * block NULL.
         */
        hsize_t      offset[2];   // hyperslab offset in the file
        hsize_t      count[2];    // size of the hyperslab in the file
        offset[0] = 1;
        offset[1] = 2;
        count[0] = NX_SUB;
        count[1] = NY_SUB;
        dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);
        /*
         * Define the memory dataspace.
         */
        hsize_t     dimsm[3];              /* memory space dimensions */
        dimsm[0] = _NX;
        dimsm[1] = _NY;
        dimsm[2] = _NZ;
        DataSpace memspace(RANK_OUT, dimsm);
        /*
         * Define memory hyperslab.
         */
        hsize_t      offset_out[3];   // hyperslab offset in memory
        hsize_t      count_out[3];    // size of the hyperslab in memory
        offset_out[0] = 3;
        offset_out[1] = 0;
        offset_out[2] = 0;
        count_out[0] = NX_SUB;
        count_out[1] = NY_SUB;
        count_out[2] = 1;
        memspace.selectHyperslab(H5S_SELECT_SET, count_out, offset_out);
        /*
         * Read data from hyperslab in the file into the hyperslab in
         * memory and display the data.
         */
        dataset.read(data_out, PredType::NATIVE_INT, memspace, dataspace);
        for (j = 0; j < _NX; j++)
        {
            for (i = 0; i < _NY; i++)
                cout << data_out[j][i][0] << " ";
            cout << endl;
        }
        /*
         * 0 0 0 0 0 0 0
         * 0 0 0 0 0 0 0
         * 0 0 0 0 0 0 0
         * 3 4 5 6 0 0 0
         * 4 5 6 7 0 0 0
         * 5 6 7 8 0 0 0
         * 0 0 0 0 0 0 0
         */
    }  // end of try block
    // catch failure caused by the H5File operations
    catch (FileIException error)
    {
        //error.printError();
        return -1;
    }
    // catch failure caused by the DataSet operations
    catch (DataSetIException error)
    {
        //error.printError();
        return -1;
    }
    // catch failure caused by the DataSpace operations
    catch (DataSpaceIException error)
    {
        //error.printError();
        return -1;
    }
    // catch failure caused by the DataSpace operations
    catch (DataTypeIException error)
    {
        //error.printError();
        return -1;
    }
    return 0;  // successfully terminated
}

int ReadGroup() {

    //H5File file("SystemResponse.h5",H5F_ACC_RDONLY);
    //Group* ggroup = new Group(file.openGroup("Animation"));


    hid_t h5f = H5Fopen("SystemResponse.h5", H5F_ACC_RDONLY, H5P_DEFAULT);

    hid_t group, subgroup, gcp1, dataset, datatype;
    herr_t status;
    H5F_info_t finfo;
    H5G_info_t ginfo;
    ssize_t size;
    hsize_t i, dims;
    char* name;
    double* data;

    status = H5Fget_info(h5f, &finfo);

    group = H5Gopen(h5f, "Animation", H5P_DEFAULT);
    status = H5Gget_info(group, &ginfo);
    //如果有多个组
	for (i = 0; i < ginfo.nlinks; i++)
	{
		size = 1 + H5Lget_name_by_idx(group, ".", H5_INDEX_NAME, H5_ITER_INC, i, NULL, 0, H5P_DEFAULT);
		name = (char*)malloc(size);
		size = 1 + H5Lget_name_by_idx(group, ".", H5_INDEX_NAME, H5_ITER_INC, i, name, (size_t)size, H5P_DEFAULT);

	}
    //单个组
    //i = 0;
    //size = 1 + H5Lget_name_by_idx(group, ".", H5_INDEX_NAME, H5_ITER_INC, i, NULL, 0, H5P_DEFAULT);
    //name = (char*)malloc(size);
    //size = 1 + H5Lget_name_by_idx(group, ".", H5_INDEX_NAME, H5_ITER_INC, i, name, (size_t)size, H5P_DEFAULT);

    //printf("Index:%d,group: %s\n", i, name);

    //group = H5Gopen(group, name, H5P_DEFAULT);
    //获取datatype dims
    dataset = H5Dopen(group, "0", H5P_DEFAULT);
    hid_t dataspace = H5Dget_space(dataset);
    datatype = H5Dget_type(dataset);
    H5Sget_simple_extent_dims(dataspace, &dims, NULL);
    //dims为数据大小（序列数据）
    //若为其他类型数据，则需指定维度，根据维度读出
    double* ddata = (double*)malloc(sizeof(double) * dims);

    cout << dims << endl;


    H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, ddata);

    for (size_t i = 0; i < dims; i++)
    {
        cout << ddata[i] << endl;
    }

    H5Dclose(dataset);
    H5Gclose(group);
    H5Fclose(h5f);


    return 0;

}

int CppReadGroup() 
{
	H5File file("SystemResponse.h5",H5F_ACC_RDONLY);

    for (int i = 0; i < file.getNumObjs(); i++)
    {
        char objName[OBJ_NAME_LEN];
        ssize_t name_size = file.getObjnameByIdx(i,objName, OBJ_NAME_LEN);
        if (name_size <= 0)
        {
            continue;
        }
        cout << "--------- objName=" << objName << endl;
        H5O_type_t type = file.childObjType(objName);

        switch (type)
        {
        case H5O_TYPE_UNKNOWN:
            break;
        case H5O_TYPE_GROUP:
        {
            
        
        
        } break;
        case H5O_TYPE_DATASET:
            break;
        case H5O_TYPE_NAMED_DATATYPE:
            break;
        case H5O_TYPE_MAP:
            break;
        case H5O_TYPE_NTYPES:
            break;
        default:
            break;
        }
    }

	Group group = file.openGroup("Animation");
    DataSet dataset = group.openDataSet("0");

    H5T_class_t type_class = dataset.getTypeClass();

	hsize_t count[1], start[1];
	start[0] = 0;
	count[0] = 77;

	DataSpace fspace = dataset.getSpace();

    int ndims = fspace.getSimpleExtentNdims();

    hsize_t dims_out[1];
    int dims = fspace.getSimpleExtentDims(dims_out,NULL);
    const int DIM = dims_out[0];



    hsize_t mdim[] = { DIM };
    DataSpace mspace(1, mdim);
    mspace.selectHyperslab(H5S_SELECT_SET, count, start, NULL, NULL);

    float* outdata = new float[DIM];

    dataset.read(outdata, PredType::NATIVE_FLOAT, mspace, fspace);

	for (int i = 0; i < DIM; i++)
	{
		cout << outdata[i] << endl;
	}

    delete[] outdata;
    outdata = NULL;
	return 0;

}