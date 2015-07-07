#include "coreneuron/nrnoc/multicore.h"
#ifdef _OPENACC
#include<openacc.h>
#endif

/* note: threads here are corresponding to global nrn_threads array */
void setup_nrnthreads_on_device(NrnThread *threads, int nthreads)  {

#ifdef _OPENACC

    int i;
    
    printf("\n --- Copying to Device! --- ");

    /* pointers for data struct on device, starting with d_ */

    for( i = 0; i < nthreads; i++) {

        NrnThread * nt = threads + i;
        NrnThread *d_nt;                //NrnThread on device
        
        double *d__data;                // nrn_threads->_data on device
        
        printf("\n -----------COPYING %d'th NrnThread TO DEVICE --------------- \n", i);

        /* -- copy NrnThread to device -- */
        d_nt = (NrnThread *) acc_copyin(nt, sizeof(NrnThread));

        /* -- copy _data to device -- */

        /*copy all double data for thread */
        d__data = (double *) acc_copyin(nt->_data, nt->_ndata*sizeof(double));

        /*update d_nt._data to point to device copy */
        acc_memcpy_to_device(&(d_nt->_data), &d__data, sizeof(double*));

        /* -- setup rhs, d, a, b, v, node_aread to point to device copy -- */
        double *dptr;

        dptr = d__data + 0*nt->end;
        acc_memcpy_to_device(&(d_nt->_actual_rhs), &(dptr), sizeof(double*));

        dptr = d__data + 1*nt->end;
        acc_memcpy_to_device(&(d_nt->_actual_d), &(dptr), sizeof(double*));

        dptr = d__data + 2*nt->end;
        acc_memcpy_to_device(&(d_nt->_actual_a), &(dptr), sizeof(double*));

        dptr = d__data + 3*nt->end;
        acc_memcpy_to_device(&(d_nt->_actual_b), &(dptr), sizeof(double*));

        dptr = d__data + 4*nt->end;
        acc_memcpy_to_device(&(d_nt->_actual_v), &(dptr), sizeof(double*));

        dptr = d__data + 5*nt->end;
        acc_memcpy_to_device(&(d_nt->_actual_area), &(dptr), sizeof(double*));

        /* @todo: nt._ml_list[tml->index] = tml->ml; */


        /* -- copy NrnThreadMembList list ml to device -- */

        NrnThreadMembList* tml;
        NrnThreadMembList* d_tml;
        NrnThreadMembList* d_last_tml;

        Memb_list * d_ml;
        int first_tml = 1;
        size_t offset = 6 * nt->end;

        for (tml = nt->tml; tml; tml = tml->next) {

            /*copy tml to device*/
            /*QUESTIONS: does tml will point to NULL as in host ? : I assume so!*/
            d_tml = (NrnThreadMembList *) acc_copyin(tml, sizeof(NrnThreadMembList));
            
            /*first tml is pointed by nt */
            if(first_tml) {
                acc_memcpy_to_device(&(d_nt->tml), &d_tml, sizeof(NrnThreadMembList*));
                first_tml = 0;
            } else {
            /*rest of tml forms linked list */
                acc_memcpy_to_device(&(d_last_tml->next), &d_tml, sizeof(NrnThreadMembList*));
            }
            
            //book keeping for linked-list 
            d_last_tml = d_tml;

            /* now for every tml, there is a ml. copy that and setup pointer */
            d_ml = (Memb_list *) acc_copyin(tml->ml, sizeof(Memb_list));
            acc_memcpy_to_device(&(d_tml->ml), &d_ml, sizeof(Memb_list*));


            dptr = d__data+offset;

            acc_memcpy_to_device(&(d_ml->data), &(dptr), sizeof(double*));
            
            int type = tml->index;
            int n = tml->ml->nodecount;
            int szp = nrn_prop_param_size_[type];
            int szdp = nrn_prop_dparam_size_[type];
            int is_art = nrn_is_artificial_[type];

            offset += n*szp;

            if (!is_art) {
                int * d_nodeindices = (int *) acc_copyin(tml->ml->nodeindices, sizeof(int)*n);
                acc_memcpy_to_device(&(d_ml->nodeindices), &d_nodeindices, sizeof(int*));
            }

            if (szdp) {
                int * d_pdata = (int *) acc_copyin(tml->ml->pdata, sizeof(int)*n*szdp);
                acc_memcpy_to_device(&(d_ml->pdata), &d_pdata, sizeof(int*));
            }

        }

        if(nt->shadow_rhs_cnt) {
            double * d_shadow_ptr;

            /* copy shadow_rhs to device and fix-up the pointer */
            d_shadow_ptr = (double *) acc_copyin(nt->_shadow_rhs, nt->shadow_rhs_cnt*sizeof(double));
            acc_memcpy_to_device(&(d_nt->_shadow_rhs), &d_shadow_ptr, sizeof(double*));

            /* copy shadow_d to device and fix-up the pointer */
            d_shadow_ptr = (double *) acc_copyin(nt->_shadow_d, nt->shadow_rhs_cnt*sizeof(double));
            acc_memcpy_to_device(&(d_nt->_shadow_d), &d_shadow_ptr, sizeof(double*));
        }
    }
#endif
}

void update_nrnthreads_on_host(NrnThread *threads, int nthreads) {

#ifdef _OPENACC
        printf("\n --- Copying to Host! --- \n");

    int i;
    
    for( i = 0; i < nthreads; i++) {

        NrnThread * nt = threads + i;
        
        /* -- copy data to host -- */

        int ne = nt->end;

        acc_update_self(nt->_actual_rhs, ne*sizeof(double));
        acc_update_self(nt->_actual_d, ne*sizeof(double));
        acc_update_self(nt->_actual_a, ne*sizeof(double));
        acc_update_self(nt->_actual_b, ne*sizeof(double));
        acc_update_self(nt->_actual_v, ne*sizeof(double));
        acc_update_self(nt->_actual_area, ne*sizeof(double));

        /* @todo: nt._ml_list[tml->index] = tml->ml; */

        /* -- copy NrnThreadMembList list ml to host -- */
        NrnThreadMembList* tml;
        for (tml = nt->tml; tml; tml = tml->next) {

          Memb_list *ml = tml->ml;
          int type = tml->index;
          int n = ml->nodecount;
          int szp = nrn_prop_param_size_[type];
          int szdp = nrn_prop_dparam_size_[type];
          int is_art = nrn_is_artificial_[type];

          acc_update_self(ml->data, n*szp*sizeof(double));

          if (!is_art) {
              acc_update_self(ml->nodeindices, n*sizeof(int));
          }

          if (szdp) {
              acc_update_self(ml->pdata, n*szdp*sizeof(int));
          }

        }

        if(nt->shadow_rhs_cnt) {
            /* copy shadow_rhs to host */
            acc_update_self(nt->_shadow_rhs, nt->shadow_rhs_cnt*sizeof(double));
            /* copy shadow_d to host */
            acc_update_self(nt->_shadow_d, nt->shadow_rhs_cnt*sizeof(double));
        }
    }
#endif

}


void modify_data_on_device(NrnThread *threads, int nthreads) {

#ifdef _OPENACC
        printf("\n --- Modifying data on device! --- \n");
#endif

    int i, j;
    
    for( i = 0; i < nthreads; i++) {

        NrnThread * nt = threads + i;
        
        /* -- modify data on device -- */

        int ne = nt->end;

        #pragma acc parallel loop present(nt[0:1])
        for (j = 0; j < ne; ++j)
        {
            nt->_actual_rhs[j] += 0.1;
            nt->_actual_d[j] += 0.1;
            nt->_actual_a[j] += 0.1;
            nt->_actual_b[j] += 0.1;
            nt->_actual_v[j] += 0.1;
            nt->_actual_area[j] += 0.1;
        }

        /* @todo: nt._ml_list[tml->index] = tml->ml; */

        NrnThreadMembList* tml;
        for (tml = nt->tml; tml; tml = tml->next) {

          Memb_list *ml = tml->ml;
          int type = tml->index;
          int n = ml->nodecount;
          int szp = nrn_prop_param_size_[type];
          int szdp = nrn_prop_dparam_size_[type];
          int is_art = nrn_is_artificial_[type];

          #pragma acc parallel loop present(ml[0:1])
          for (j = 0; j < n*szp; ++j)
          {
            ml->data[j] += 0.1; 
          }


          if (!is_art) {
            #pragma acc parallel loop present(ml[0:1])
            for (j = 0; j < n; ++j)
            {
              ml->nodeindices[j] += 1;
            }
          }

          if (szdp) {
            #pragma acc parallel loop present(ml[0:1])
            for (j = 0; j < n*szdp; ++j)
            {
              ml->pdata[j] += 1;
            }
          }
        }    

        if(nt->shadow_rhs_cnt) {
            #pragma acc parallel loop present(nt[0:1])
            for (j = 0; j < nt->shadow_rhs_cnt; ++j)
            {
              nt->_shadow_rhs[j] += 0.1;
              nt->_shadow_d[j] += 0.1;
            }
        }
    }
}


void write_iarray_to_file(FILE *hFile, int *data, int n) {
    int i;

    for(i=0; i<n; i++) {
        fprintf(hFile, "%d\n", data[i]);
    }
    fprintf(hFile, "---\n");
}

void write_darray_to_file(FILE *hFile, double *data, int n) {
    int i;

    for(i=0; i<n; i++) {
        fprintf(hFile, "%lf\n", data[i]);
    }
    fprintf(hFile, "---\n");
}

void write_nodeindex_to_file(int gid, int id, int *data, int n) {
    int i;
    FILE *hFile;
    char filename[4096];

    sprintf(filename, "%d.%d.index", gid, id);
    hFile = fopen(filename, "w");

    for(i=0; i<n; i++) {
        fprintf(hFile, "%d\n", data[i]);
    }

    fclose(hFile);
}


void dump_nt_to_file(char *filename, NrnThread *threads, int nthreads) {
 
    FILE *hFile;
    int i, j;
    
    NrnThreadMembList* tml;


    for( i = 0; i < nthreads; i++) {

      char fname[1024]; 
      sprintf(fname, "%s%d.dat", filename, i);
      hFile = fopen(fname, "w");

      NrnThread * nt = threads + i;
        

      long int offset;
      int nmech = 0;


      int ne = nt->end;
      fprintf(hFile, "%d\n", nt->_ndata);
      write_darray_to_file(hFile, nt->_data, nt->_ndata);

      fprintf(hFile, "%d\n", nt->end);
      for (tml = nt->tml; tml; tml = tml->next, nmech++);
      fprintf(hFile, "%d\n", nmech);

      offset = nt->end*6;

      for (tml = nt->tml; tml; tml = tml->next) {
        int type = tml->index;
        int is_art = nrn_is_artificial_[type];
        Memb_list* ml = tml->ml;
        int n = ml->nodecount;
        int szp = nrn_prop_param_size_[type];
        int szdp = nrn_prop_dparam_size_[type];

        fprintf(hFile, "%d %d %d %d %d %ld\n", type, is_art, n, szp, szdp, offset);
        offset += n*szp;

        if (!is_art) {
            write_iarray_to_file(hFile, ml->nodeindices, ml->nodecount);
//            write_nodeindex_to_file(gid, type, ml->nodeindices, ml->nodecount);
        }

        if (szdp) {
            write_iarray_to_file(hFile, ml->pdata, n*szdp);
        }
      }

      if(nt->shadow_rhs_cnt) {
        write_darray_to_file(hFile, nt->_shadow_rhs, nt->shadow_rhs_cnt);
        write_darray_to_file(hFile, nt->_shadow_d, nt->shadow_rhs_cnt);
      }

      fclose(hFile);

    }
}

void finalize_data_on_device(NrnThread *, int nthreads) {
   #ifdef _OPENACC
        acc_shutdown ( acc_device_default);
    #endif
}
