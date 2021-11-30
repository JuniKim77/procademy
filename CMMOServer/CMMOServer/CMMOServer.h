#pragma once

#include "CSession.h"

namespace procademy
{
	class CMMOServer
	{
	protected:
		CMMOServer();
		virtual ~CMMOServer();
		bool Start();
		void Stop();
		void LoadInitFile(const WCHAR* fileName);
		void QuitServer();
		void SetZeroCopy(bool on);
		void SetNagle(bool on);


	private:

	};
}