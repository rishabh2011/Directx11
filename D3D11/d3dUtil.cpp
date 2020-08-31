#include "D3DUtil.h"

d3dException::d3dException(HRESULT hr, const std::wstring& functionName, const std::wstring& fileName, int line) :
	errorCode{hr}, functionName{functionName}, fileName{fileName}, line{line}
{}

std::wstring d3dException::toString() const
{
	_com_error err(errorCode);
	std::wstring errormsg = err.ErrorMessage();
	return functionName + L" failed in " + fileName + L"; line " + std::to_wstring(line) + L"; error : " + errormsg;
}