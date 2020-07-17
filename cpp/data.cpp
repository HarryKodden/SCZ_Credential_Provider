#include "data.h"

namespace Data
{

	namespace Gui
	{

		GUI*& Get()
		{
			static struct GUI *data = NULL;

			return data;
		}

		void Default()
		{
			struct GUI*& data = Get();

			if (data == NULL)
				return;

			ZERO(data->user_name);
			ZERO(data->domain_name);
			ZERO(data->token);
		}

		void Init()
		{
			struct GUI*& data = Get();
			
			if (data == NULL)
				data = (struct GUI*) malloc(sizeof(struct GUI));

			Default();
		}

		void Deinit()
		{
			struct GUI*& data = Get();

			Default();

			if (data != NULL)
			{
				free(data);
				data = NULL;
			}
		}

	} // Namespace Gui

	namespace Provider
	{

		PROVIDER*& Get()
		{
			static struct PROVIDER *data = NULL;

			return data;
		}

		void Default()
		{
			struct PROVIDER*& data = Get();

			if (data == NULL)
				return;

			data->_pcpe = NULL;
			data->_upAdviseContext = NULL;
			data->credPackFlags = 0;
		}

		void Init()
		{
			struct PROVIDER*& data = Get();

			data = (struct PROVIDER*) malloc(sizeof(struct PROVIDER));

			Default();
		}

		void Deinit()
		{
			struct PROVIDER*& data = Get();

			Default();

			free(data);
			data = NULL;
		}

	} // Namespace Provider

	namespace Credential
	{

		CREDENTIAL*& Get()
		{
			static struct CREDENTIAL *data = NULL;

			return data;
		}

		void Default()
		{
			struct CREDENTIAL*& data = Get();

			if (data == NULL)
				return;

			data->user_name = NULL;
			data->domain_name = NULL;
			data->ldap_password = NULL;

			data->pqcws = NULL;
		}

		void Init()
		{
			struct CREDENTIAL*& data = Get();

			data = (struct CREDENTIAL*) malloc(sizeof(struct CREDENTIAL));

			Default();
		}

		void Deinit()
		{
			struct CREDENTIAL*& data = Get();

			Default();

			free(data);
			data = NULL;
		}

	} // Namespace Credential

} // Namespace Data