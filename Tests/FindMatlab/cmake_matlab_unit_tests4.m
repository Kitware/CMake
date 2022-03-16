classdef cmake_matlab_unit_tests4 < matlab.unittest.TestCase
  % Testing R2017b and R2018a APIs
  properties
  end

  methods (Test)
    function testR2017b(testCase)
      ret = cmake_matlab_mex2a(5+6i);
      testCase.verifyEqual(ret, 8);
    end

    function testR2018a(testCase)
      ret = cmake_matlab_mex2b(5+6i);
      if not(verLessThan('matlab','9.4')) % R2018a
        testCase.verifyEqual(ret, 16);
        disp('TESTING version >= 9.4')
      else
        testCase.verifyEqual(ret, 8);
      end
    end

  end
end
