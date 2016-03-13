#include "param.h"
#include "util.h"

#include <regex>

//shamelessly stolen from http://stackoverflow.com/questions/554013/regular-expression-to-split-on-spaces-unless-in-quotes
static const char *REGEX_PARAM = "[^\\s\"']+|\"([^\"]*)\"|'([^']*)'";

static const std::regex __param_regex(REGEX_PARAM);

static int __param_init(struct param *param)
{
	param->input_string[strcspn(param->input_string, "\r\n")] = '\0';

	std::string input_string = param->input_string;
	std::sregex_iterator params_begin(input_string.begin(), input_string.end(), __param_regex);
	std::sregex_iterator params_end = std::sregex_iterator();
	int match_index = 0;

	param->num_parameters = std::distance(params_begin, params_end);

	//if (param->num_parameters > 1) {
	//	/*
	//	 * Discard the end-most match as it seems to be rubbish.
	//	 *
	//	 * TODO: Find out why this is
	//	 */
	//	param->num_parameters--;
	//}

	param->parameters = talloc_array(param, char *, param->num_parameters);
	if (param->parameters == NULL) {
		_ERROR("%s: Cannot allocate space for %d matches in params.\n", __FUNCTION__, param->num_parameters);
		return -ENOMEM;
	}

	for (std::sregex_iterator i = params_begin; i != params_end; ++i) {
		std::smatch match = *i;

		if (match_index == param->num_parameters) {
			break;
		}

		param->parameters[match_index] = talloc_strdup(param->parameters, match.str().c_str());
		if (param->parameters[match_index] == NULL) {
			_ERROR("%s: Out of memory copying regex match into parameters.\n", __FUNCTION__);
			return -ENOMEM;
		}

		match_index++;
	}

	return 0;
}

int param_new(TALLOC_CTX *context, const char *input_string, struct param **out_param)
{
	int ret = -1;
	TALLOC_CTX *temp_context;
	struct param *param;

	temp_context = talloc_new(NULL);
	if (temp_context == NULL) {
		_ERROR("%s: out of memory allocating param context.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	param = talloc(temp_context, struct param);
	if (param == NULL) {
		_ERROR("%s: out of memory allocating param context.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	param->input_string = talloc_strdup(param, input_string);
	if (param->input_string == NULL) {
		_ERROR("%s: out of memory copying input string to param context.\n", __FUNCTION__);
		ret = -ENOMEM;
		goto out;
	}

	__param_init(param);

	*out_param = (struct param *)talloc_steal(context, param);

	ret = 0;
out:
	talloc_free(temp_context);

	return ret;
}

void param_free(struct param *param)
{
	talloc_free(param);
}