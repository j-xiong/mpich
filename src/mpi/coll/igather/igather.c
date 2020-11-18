/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpiimpl.h"

/*
=== BEGIN_MPI_T_CVAR_INFO_BLOCK ===

cvars:
    - name        : MPIR_CVAR_IGATHER_INTRA_ALGORITHM
      category    : COLLECTIVE
      type        : enum
      default     : auto
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select igather algorithm
        auto - Internal algorithm selection (can be overridden with MPIR_CVAR_COLL_SELECTION_TUNING_JSON_FILE)
        sched_auto - Internal algorithm selection for sched-based algorithms
        sched_binomial     - Force binomial algorithm
        gentran_tree       - Force genetric transport based tree algorithm

    - name        : MPIR_CVAR_IGATHER_TREE_KVAL
      category    : COLLECTIVE
      type        : int
      default     : 2
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        k value for tree based igather

    - name        : MPIR_CVAR_IGATHER_INTER_ALGORITHM
      category    : COLLECTIVE
      type        : enum
      default     : auto
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select igather algorithm
        auto - Internal algorithm selection (can be overridden with MPIR_CVAR_COLL_SELECTION_TUNING_JSON_FILE)
        sched_auto - Internal algorithm selection for sched-based algorithms
        sched_long  - Force long inter algorithm
        sched_short - Force short inter algorithm

    - name        : MPIR_CVAR_IGATHER_DEVICE_COLLECTIVE
      category    : COLLECTIVE
      type        : boolean
      default     : true
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This CVAR is only used when MPIR_CVAR_DEVICE_COLLECTIVES
        is set to "percoll".  If set to true, MPI_Igather will
        allow the device to override the MPIR-level collective
        algorithms.  The device might still call the MPIR-level
        algorithms manually.  If set to false, the device-override
        will be disabled.

=== END_MPI_T_CVAR_INFO_BLOCK ===
*/

int MPIR_Igather_allcomm_auto(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                              void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
                              MPIR_Comm * comm_ptr, MPIR_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_Csel_coll_sig_s coll_sig = {
        .coll_type = MPIR_CSEL_COLL_TYPE__IGATHER,
        .comm_ptr = comm_ptr,

        .u.igather.sendbuf = sendbuf,
        .u.igather.sendcount = sendcount,
        .u.igather.sendtype = sendtype,
        .u.igather.recvcount = recvcount,
        .u.igather.recvbuf = recvbuf,
        .u.igather.recvtype = recvtype,
        .u.igather.root = root,
    };

    MPII_Csel_container_s *cnt = MPIR_Csel_search(comm_ptr->csel_comm, coll_sig);
    MPIR_Assert(cnt);

    switch (cnt->id) {
        case MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIR_Igather_intra_gentran_tree:
            mpi_errno =
                MPIR_Igather_intra_gentran_tree(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                                                recvtype, root, comm_ptr,
                                                cnt->u.igather.intra_gentran_tree.k, request);
            break;

        case MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIR_Igather_intra_sched_auto:
            MPII_SCHED_WRAPPER(MPIR_Igather_intra_sched_auto, comm_ptr, request, sendbuf, sendcount,
                               sendtype, recvbuf, recvcount, recvtype, root);
            break;

        case MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIR_Igather_intra_sched_binomial:
            MPII_SCHED_WRAPPER(MPIR_Igather_intra_sched_binomial, comm_ptr, request, sendbuf,
                               sendcount, sendtype, recvbuf, recvcount, recvtype, root);
            break;

        case MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIR_Igather_inter_sched_auto:
            MPII_SCHED_WRAPPER(MPIR_Igather_inter_sched_auto, comm_ptr, request, sendbuf, sendcount,
                               sendtype, recvbuf, recvcount, recvtype, root);
            break;

        case MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIR_Igather_inter_sched_long:
            MPII_SCHED_WRAPPER(MPIR_Igather_inter_sched_long, comm_ptr, request, sendbuf, sendcount,
                               sendtype, recvbuf, recvcount, recvtype, root);
            break;

        case MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIR_Igather_inter_sched_short:
            MPII_SCHED_WRAPPER(MPIR_Igather_inter_sched_short, comm_ptr, request, sendbuf,
                               sendcount, sendtype, recvbuf, recvcount, recvtype, root);
            break;

        default:
            MPIR_Assert(0);
    }

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

int MPIR_Igather_intra_sched_auto(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                  void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
                                  MPIR_Comm * comm_ptr, MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;

    mpi_errno =
        MPIR_Igather_intra_sched_binomial(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                                          recvtype, root, comm_ptr, s);
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

int MPIR_Igather_inter_sched_auto(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                                  void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
                                  MPIR_Comm * comm_ptr, MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;
    MPI_Aint local_size, remote_size;
    MPI_Aint recvtype_size, sendtype_size, nbytes;

    if (root == MPI_PROC_NULL) {
        /* local processes other than root do nothing */
        goto fn_exit;
    }

    remote_size = comm_ptr->remote_size;
    local_size = comm_ptr->local_size;

    if (root == MPI_ROOT) {
        MPIR_Datatype_get_size_macro(recvtype, recvtype_size);
        nbytes = recvtype_size * recvcount * remote_size;
    } else {
        /* remote side */
        MPIR_Datatype_get_size_macro(sendtype, sendtype_size);
        nbytes = sendtype_size * sendcount * local_size;
    }

    if (nbytes < MPIR_CVAR_GATHER_INTER_SHORT_MSG_SIZE) {
        mpi_errno =
            MPIR_Igather_inter_sched_short(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                                           recvtype, root, comm_ptr, s);
    } else {
        mpi_errno =
            MPIR_Igather_inter_sched_long(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                                          recvtype, root, comm_ptr, s);
    }

  fn_exit:
    return mpi_errno;
}

int MPIR_Igather_sched_auto(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                            void *recvbuf, int recvcount, MPI_Datatype recvtype,
                            int root, MPIR_Comm * comm_ptr, MPIR_Sched_t s)
{
    int mpi_errno = MPI_SUCCESS;

    if (comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        mpi_errno = MPIR_Igather_intra_sched_auto(sendbuf, sendcount, sendtype,
                                                  recvbuf, recvcount, recvtype, root, comm_ptr, s);
    } else {
        mpi_errno = MPIR_Igather_inter_sched_auto(sendbuf, sendcount, sendtype, recvbuf,
                                                  recvcount, recvtype, root, comm_ptr, s);
    }

    return mpi_errno;
}

int MPIR_Igather_impl(const void *sendbuf, int sendcount,
                      MPI_Datatype sendtype, void *recvbuf, int recvcount,
                      MPI_Datatype recvtype, int root, MPIR_Comm * comm_ptr,
                      MPIR_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;

    *request = NULL;

    /* If the user picks one of the transport-enabled algorithms, branch there
     * before going down to the MPIR_Sched-based algorithms. */
    /* TODO - Eventually the intention is to replace all of the
     * MPIR_Sched-based algorithms with transport-enabled algorithms, but that
     * will require sufficient performance testing and replacement algorithms. */
    if (comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        /* intracommunicator */
        switch (MPIR_CVAR_IGATHER_INTRA_ALGORITHM) {
            case MPIR_CVAR_IGATHER_INTRA_ALGORITHM_gentran_tree:
                mpi_errno =
                    MPIR_Igather_intra_gentran_tree(sendbuf, sendcount, sendtype,
                                                    recvbuf, recvcount, recvtype, root, comm_ptr,
                                                    MPIR_CVAR_IGATHER_TREE_KVAL, request);
                break;

            case MPIR_CVAR_IGATHER_INTRA_ALGORITHM_sched_binomial:
                MPII_SCHED_WRAPPER(MPIR_Igather_intra_sched_binomial, comm_ptr, request, sendbuf,
                                   sendcount, sendtype, recvbuf, recvcount, recvtype, root);
                break;

            case MPIR_CVAR_IGATHER_INTRA_ALGORITHM_sched_auto:
                MPII_SCHED_WRAPPER(MPIR_Igather_intra_sched_auto, comm_ptr, request, sendbuf,
                                   sendcount, sendtype, recvbuf, recvcount, recvtype, root);
                break;

            case MPIR_CVAR_IGATHER_INTRA_ALGORITHM_auto:
                mpi_errno =
                    MPIR_Igather_allcomm_auto(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                                              recvtype, root, comm_ptr, request);
                break;

            default:
                MPIR_Assert(0);
        }
    } else {
        switch (MPIR_CVAR_IGATHER_INTER_ALGORITHM) {
            case MPIR_CVAR_IGATHER_INTER_ALGORITHM_sched_long:
                MPII_SCHED_WRAPPER(MPIR_Igather_inter_sched_long, comm_ptr, request, sendbuf,
                                   sendcount, sendtype, recvbuf, recvcount, recvtype, root);
                break;

            case MPIR_CVAR_IGATHER_INTER_ALGORITHM_sched_short:
                MPII_SCHED_WRAPPER(MPIR_Igather_inter_sched_short, comm_ptr, request, sendbuf,
                                   sendcount, sendtype, recvbuf, recvcount, recvtype, root);
                break;

            case MPIR_CVAR_IGATHER_INTER_ALGORITHM_sched_auto:
                MPII_SCHED_WRAPPER(MPIR_Igather_inter_sched_auto, comm_ptr, request, sendbuf,
                                   sendcount, sendtype, recvbuf, recvcount, recvtype, root);
                break;

            case MPIR_CVAR_IGATHER_INTER_ALGORITHM_auto:
                mpi_errno =
                    MPIR_Igather_allcomm_auto(sendbuf, sendcount, sendtype, recvbuf, recvcount,
                                              recvtype, root, comm_ptr, request);
                break;

            default:
                MPIR_Assert(0);
        }
    }

    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

int MPIR_Igather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype, int root,
                 MPIR_Comm * comm_ptr, MPIR_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;

    if ((MPIR_CVAR_DEVICE_COLLECTIVES == MPIR_CVAR_DEVICE_COLLECTIVES_all) ||
        ((MPIR_CVAR_DEVICE_COLLECTIVES == MPIR_CVAR_DEVICE_COLLECTIVES_percoll) &&
         MPIR_CVAR_IGATHER_DEVICE_COLLECTIVE)) {
        mpi_errno =
            MPID_Igather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm_ptr,
                         request);
    } else {
        mpi_errno = MPIR_Igather_impl(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype,
                                      root, comm_ptr, request);
    }

    return mpi_errno;
}
