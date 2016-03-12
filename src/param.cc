#include "param.h"
#include "util.h"

#include <regex>

const char *REGEX_PARAM = "";

int param_context_new(TALLOC_CTX *context, const char *input_string, struct param_context **out_context)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct param_context *param;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating param context.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	/*param = (struct param_context *)talloc(struct param_context);
	if (param == NULL) {

	}*/

out:
	talloc_free(temp_context);

	return ret;
}