// HDF5Parser.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "hdf5.h"
#include "H5Cpp.h"
#include "H5File.h"

using namespace std;
using namespace H5;
using std::cout;
using std::endl;

#define H5FILE_NAME "SDS.h5"
#define DATASETNAME "IntArray"
#define NX		5	/* dataset dimensions */
#define NY      6
#define RANK    2
int Test() {

	hid_t       file, dataset;         /* file and dataset handles */
	hid_t       datatype, dataspace;   /* handles */
	hsize_t     dimsf[2];              /* dataset dimensions */
	herr_t      status;
	int         data[NX][NY];          /* data to write */
	int         i, j;

	/*
	 * Data  and output buffer initialization.
	 */
	for (j = 0; j < NX; j++)
	{
		for (i = 0; i < NY; i++)
		{
			data[j][i] = i + j;
		}
	}

	/*
	 * 0 1 2 3 4 5
	 * 1 2 3 4 5 6
	 * 2 3 4 5 6 7
	 * 3 4 5 6 7 8
	 * 4 5 6 7 8 9
	 */

	 /*
	  * Create a new file using H5F_ACC_TRUNC access,
	  * default file creation properties, and default file
	  * access properties.
	  */
	file = H5Fcreate(H5FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

	/*
	 * Describe the size of the array and create the data space for fixed
	 * size dataset.
	 */
	dimsf[0] = NX;
	dimsf[1] = NY;
	dataspace = H5Screate_simple(RANK, dimsf, NULL);

	/*
	 * Define datatype for the data in the file.
	 * We will store little endian INT numbers.
	 */
	datatype = H5Tcopy(H5T_NATIVE_INT);
	status = H5Tset_order(datatype, H5T_ORDER_LE);

	/*
	 * Create a new dataset within the file using defined dataspace and
	 * datatype and default dataset creation properties.
	 */
	dataset = H5Dcreate2(file, DATASETNAME, datatype, dataspace,
		H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	/*
	 * Write the data to the dataset using default transfer properties.
	 */
	status = H5Dwrite(dataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

	/*
	 * Close/release resources.
	 */
	H5Sclose(dataspace);
	H5Tclose(datatype);
	H5Dclose(dataset);
	H5Fclose(file);

	return 0;

}

const char* fileName = "Lee.h5";
const char* datasetName = "Matrix";
const int   MSPACE1_RANK = 1;   // Rank of the first dataset in memory
const int   MSPACE1_DIM = 50;   // Dataset size in memory
const int   MSPACE2_RANK = 1;   // Rank of the second dataset in memory
const int   MSPACE2_DIM = 4;    // Dataset size in memory
const int   FSPACE_RANK = 2;    // Dataset rank as it is stored in the file
const int   FSPACE_DIM1 = 8;    // Dimension sizes of the dataset as it is
const int   FSPACE_DIM2 = 12;   //  stored in the file
const int   MSPACE_RANK = 2;    // Rank of the first dataset in memory
const int   MSPACE_DIM1 = 8;    // We will read dataset back from the file
const int   MSPACE_DIM2 = 12;    //  to the dataset in memory with these
				//  dataspace parameters
const int   NPOINTS = 4;    // Number of points that will be selected
				//  and overwritten
int HDF5Write() {

    int   i, j; // loop indices */
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
         * Create a file.
         */
        H5File* file = new H5File(fileName, H5F_ACC_TRUNC);
        /*
        * Create property list for a dataset and set up fill values.
        */
        int fillvalue = 0;   /* Fill value for the dataset */
        DSetCreatPropList plist;
        plist.setFillValue(PredType::NATIVE_INT, &fillvalue);
        /*
         * Create dataspace for the dataset in the file.
         */
        hsize_t fdim[] = { 4, 5, 9 }; // dim sizes of ds (on disk)
        DataSpace fspace(3, fdim);
        /*
         * Create dataset and write it into the file.
         */
        DataSet* dataset = new DataSet(file->createDataSet(datasetName, PredType::NATIVE_INT, fspace, plist));
        /*
         * Select hyperslab for the dataset in the file, using 3x2 blocks,
         * (4,3) stride and (2,4) count starting at the position (0,1).
         */ 
        hsize_t start[2]; // Start of hyperslab
        hsize_t stride[2]; // Stride of hyperslab
        hsize_t count[2];  // Block count
        hsize_t block[2];  // Block sizes
        start[0] = 0; start[1] = 1;
        stride[0] = 4; stride[1] = 3;
        count[0] = 2; count[1] = 4;
        block[0] = 3; block[1] = 2;
        fspace.selectHyperslab(H5S_SELECT_SET, count, start, stride, block);
        /*
         * Create dataspace for the first dataset.
         */
        hsize_t dim1[] = { MSPACE1_DIM };  /* Dimension size of the first dataset
                                           (in memory) */
        DataSpace mspace1(MSPACE1_RANK, dim1);
        /*
         * Select hyperslab.
         * We will use 48 elements of the vector buffer starting at the
         * second element.  Selected elements are 1 2 3 . . . 48
         */
        start[0] = 1;
        stride[0] = 1;
        count[0] = 48;
        block[0] = 1;
        mspace1.selectHyperslab(H5S_SELECT_SET, count, start, stride, block);
        /*
         * Write selection from the vector buffer to the dataset in the file.
         *
         * File dataset should look like this:
         *                    0  1  2  0  3  4  0  5  6  0  7  8
         *                    0  9 10  0 11 12  0 13 14  0 15 16
         *                    0 17 18  0 19 20  0 21 22  0 23 24
         *                    0  0  0  0  0  0  0  0  0  0  0  0
         *                    0 25 26  0 27 28  0 29 30  0 31 32
         *                    0 33 34  0 35 36  0 37 38  0 39 40
         *                    0 41 42  0 43 44  0 45 46  0 47 48
         *                    0  0  0  0  0  0  0  0  0  0  0  0
         */
        int    vector[MSPACE1_DIM]; // vector buffer for dset
        /*
         * Buffer initialization.
         */
        vector[0] = vector[MSPACE1_DIM - 1] = -1;
        for (i = 1; i < MSPACE1_DIM - 1; i++)
            vector[i] = i;
        dataset->write(vector, PredType::NATIVE_INT, mspace1, fspace);
        /*
         * Reset the selection for the file dataspace fid.
         */
        fspace.selectNone();
        /*
         * Create dataspace for the second dataset.
         */
        hsize_t dim2[] = { MSPACE2_DIM };  /* Dimension size of the second dataset
                                           (in memory */
        DataSpace mspace2(MSPACE2_RANK, dim2);
        /*
         * Select sequence of NPOINTS points in the file dataspace.
         */
        hsize_t coord[NPOINTS][FSPACE_RANK]; /* Array to store selected points
                                                from the file dataspace */
        coord[0][0] = 0; coord[0][1] = 0;
        coord[1][0] = 3; coord[1][1] = 3;
        coord[2][0] = 3; coord[2][1] = 5;
        coord[3][0] = 5; coord[3][1] = 6;
        fspace.selectElements(H5S_SELECT_SET, NPOINTS, (const hsize_t*)coord);
        /*
         * Write new selection of points to the dataset.
         */
        int    values[] = { 53, 59, 61, 67 };  /* New values to be written */
        dataset->write(values, PredType::NATIVE_INT, mspace2, fspace);
        /*
         * File dataset should look like this:
         *                   53  1  2  0  3  4  0  5  6  0  7  8
         *                    0  9 10  0 11 12  0 13 14  0 15 16
         *                    0 17 18  0 19 20  0 21 22  0 23 24
         *                    0  0  0 59  0 61  0  0  0  0  0  0
         *                    0 25 26  0 27 28  0 29 30  0 31 32
         *                    0 33 34  0 35 36 67 37 38  0 39 40
         *                    0 41 42  0 43 44  0 45 46  0 47 48
         *                    0  0  0  0  0  0  0  0  0  0  0  0
         *
         */
         /*
          * Close the dataset and the file.
          */
        delete dataset;
        delete file;

        return 0;
		cout << "========================================== read start" << endl;

        /*
         * Open the file.
         */
        file = new H5File(fileName, H5F_ACC_RDONLY);
        /*
         * Open the dataset.
         */
        dataset = new DataSet(file->openDataSet(datasetName));
        /*
         * Get dataspace of the dataset.
         */
        fspace = dataset->getSpace();
        /*
         * Select first hyperslab for the dataset in the file. The following
         * elements are selected:
         *                     10  0 11 12
         *                     18  0 19 20
         *                      0 59  0 61
         *
         */
        int rank = fspace.getSimpleExtentNdims();
        cout << "rank=" << rank << endl;

        hsize_t* dims_size = new hsize_t[rank];
        //int dims = fspace.getSimpleExtentDims(dims_size, NULL);

        const int row = dims_size[0];
        const int colum = dims_size[1];

        start[0] = 0; start[1] = 0;
        count[0] = row; count[1] = colum;

        fspace.selectHyperslab(H5S_SELECT_SET, count, start, NULL, NULL);

        /*
         * Create memory dataspace.
         */
        hsize_t mdim[] = { row, colum };
        DataSpace mspace(rank, mdim);
        /*
         * Select two hyperslabs in memory. Hyperslabs has the same
         * size and shape as the selected hyperslabs for the file dataspace.
         */
        start[0] = 0; start[1] = 0;
        count[0] = row; count[1] = colum;
        mspace.selectHyperslab(H5S_SELECT_SET, count, start, NULL, NULL);
        /*
         * Initialize data buffer.
         */
        //int matrix_out[MSPACE_DIM1][MSPACE_DIM2];
        //for (i = 0; i < MSPACE_DIM1; i++)
        //    for (j = 0; j < MSPACE_DIM2; j++)
        //        matrix_out[i][j] = 0;

        //int matrix[row][colum];

        int* matrix_out = new int[row * colum];

        /*
         * Read data back to the buffer matrix.
         */
        dataset->read(matrix_out, PredType::NATIVE_INT, mspace, fspace);
        /*
         * Display the result.  Memory dataset is:
         *
         *                    10  0 11 12  0  0  0  0  0
         *                    18  0 19 20  0 21 22  0  0
         *                     0 59  0 61  0  0  0  0  0
         *                     0  0 27 28  0 29 30  0  0
         *                     0  0 35 36 67 37 38  0  0
         *                     0  0 43 44  0 45 46  0  0
         *                     0  0  0  0  0  0  0  0  0
         *                     0  0  0  0  0  0  0  0  0
         */

        for (int i = 0; i < row; i++)
        {
            for (int j = 0;j<colum;j++)
            {
                cout << matrix_out[i * colum + j] << " ";
            }

            cout << endl;
        }
        /*
         * Close the dataset and the file.
         */
        delete dataset;
        delete file;
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
    return 0;
}

#include "Hdf5Reader.h"
#include "Hdf5Group.h"
#include "Hdf5Parser.h"
#include "StlParser.h"

void Test(vector<double>&& vec) {
    vector<double> aa;
    aa.emplace_back(1.0);
	aa.emplace_back(2.0);

    vec = aa;
}


int main()
{
    vector<double> a;
    // Test(a);

//    StlParser stlParser;
//    stlParser.readStlData("feiji.stl");

 //   HDF5Write();
 //   std::cout << "################### Hello World!\n" << endl;

	Hdf5Parser parser;

	parser.readHdf5("SystemResponse.h5"); //SystemResponse

    H5Item* item = parser.getH5Item();

    string name = item->name;
    vector<ItemNode*> nodes = item->itemVec;

    ItemNode* cons = item->itemVec[0];
    ItemNode* obj = cons->itemVec[2];
    ItemNode* subobj = obj->subVec[0];
    ItemNode* chara = subobj->itemVec[0];
    ItemNode* comp = chara->itemVec[2];

    vector<double> data = parser.getCurveDateSet(comp);
    int count = 0;
    int count2 = 0;

    vector<double> vec = parser.getTimeStamps();
    DouMap ex1 = parser.getAnimationBaseData();
    IntMap ex2 = parser.getAnimationIndex();

    parser.getAnimationIndex();

}
