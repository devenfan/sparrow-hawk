#ifndef W1_USERSERVICE_H_INCLUDED
#define W1_USERSERVICE_H_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif




typedef void w1_master_added(w1_master_id master_id);

typedef void w1_master_removed(w1_master_id master_id);


typedef void w1_slave_added(w1_slave_rn salve_rn);

typedef void w1_slave_removed(w1_slave_rn salve_rn);



typedef struct w1_user_callbacks{

    w1_master_added * master_added_callback;

    w1_master_removed * master_removed_callback;

    w1_slave_added * slave_added_callback;

    w1_slave_removed * slave_removed_callback;

}w1_user_callbacks;


typedef struct w1_user_service {

    void (*init)(w1_user_callbacks * callbacks);

    BOOL (*start)();

    void (*stop)();

    w1_master_id (*get_current_master)();

    void (*get_current_slaves)(w1_slave_rn * slaveIDs, int * slaveCount);

    //BOOL (*list_masters)(w1_master_id * masterIDs, int * masterCount);

    BOOL (*begin_exclusive)(w1_master_id masterId);

    void (*end_exclusive)(w1_master_id masterId);

    BOOL (*search_slaves)(w1_master_id masterId, w1_slave_rn * slaves, int * pSlaveCount);

    BOOL (*master_reset)(w1_master_id masterId);

    BOOL (*master_read)(w1_master_id masterId, int readLen, void * data);

    BOOL (*master_write)(w1_master_id masterId, int writeLen, void * data);

    BOOL (*master_touch)(w1_master_id masterId, void * dataIn, int dataInLen, void * dataOut, int * pDataOutLen);


} w1_user_service;



#ifdef __cplusplus
}
#endif


#endif // W1_USERSERVICE_H_INCLUDED
