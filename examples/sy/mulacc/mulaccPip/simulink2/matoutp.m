function out = matoutp( inp )
%UNTITLED4 Summary of this function goes here
%   Detailed explanation goes here

fprintf(evalin('base','fidout'),'%f\n',inp);
% fseek(evalin('base','fidout') , 0 , 'eof');
%fclose(evalin('base','fidout'));
%evalin('base','fidout = fopen(''forsyde_out'',''w'');');
out=inp;
end
