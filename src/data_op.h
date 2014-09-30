/*
 * file name: data_op.h
 * purpose  :
 * author   : huang chunping
 * date     : 2014-09-28
 * history  :
 */

#ifndef __DATA_OP_H__
#define __DATA_OP_H__

#include <string>
#include <map>
using namespace std;

typedef bool (*attr_set_func)(char *data, int data_len, void *ctx);
typedef bool (*attr_reset_func)(void *ctx);

#define DEF_SET_FUNC(struct_name, attr_name) \
	bool set_##attr_name(char *data, int data_len, void *ctx) { \
		struct_name *attr = (struct_name*)ctx;

#define DEF_RESET_FUNC(struct_name, attr_name) \
	bool reset_##attr_name(void *ctx) { \
		struct_name *attr = (struct_name*)ctx;

typedef struct {
	char 			*attr_name;
	attr_set_func 	setter;
	attr_reset_func resetter;
}DataOpInfo;

class DataOpRegistProxy;

class DataOpFactory {
public:
	~DataOpFactory() {
	}

	static DataOpFactory* inst() {
		static DataOpFactory factory;

		return &factory;
	}

	const DataOpInfo* get_ops_info(const string& class_name, const string& attr_name) {
		map<string, const DataOpInfo*>::iterator iter = class_ops.find(class_name);
		if (iter == class_ops.end()) {
			return NULL;
		}

		const DataOpInfo* op_info_array = iter->second;
		int i = 0;
		while (op_info_array[i].attr_name) {
			if (strcmp(op_info_array[i].attr_name, attr_name.c_str()) == 0) {
				return (op_info_array + i);
			}
			++i;
		}

		return NULL;
	}

	friend class DataOpRegistProxy;
private:
	DataOpFactory() {
	}
	
private:
	map<string, const DataOpInfo*> class_ops;
};

class DataOpRegistProxy {
public:
	DataOpRegistProxy(const string class_name, DataOpInfo* ops_info) {
		DataOpFactory::inst()->class_ops[class_name] = ops_info;
	}
};

#define DECLARE_CLASS_OP(class_type) \
static DataOpRegistProxy	*class_type##_reg_proxy; \
static DataOpInfo			attr_ops[];

#define IMPLEMENT_CLASS_OP(class_name, class_type) \
DataOpRegistProxy * class_type##_reg_proxy = new \
	DataOpRegistProxy(class_name, class_type::attr_ops);

#endif // __DATA_OP_H__
