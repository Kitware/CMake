
classdef cmake_matlab_unit_tests5 < matlab.unittest.TestCase
  % C++ API test
  properties
  end

  methods (Test)
    function testDummyCall(testCase)
      % very simple call test
      disp('TESTING C++')
      ret = cmake_matlab_mex3(162);
      testCase.verifyEqual(ret, 162);
    end

    function testFailTest(testCase)
      testCase.verifyError(@() cmake_matlab_mex3, 'MATLAB:mex:CppMexException');
    end

  end
end
