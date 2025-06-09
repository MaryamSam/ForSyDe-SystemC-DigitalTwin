function outp = matinp( ~ )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

persistent res;
res = [1 ,4];
outp = res;
if ~evalin('base','feof(fidin)')
    temp = fgetl (evalin('base','fidin'));
    res = sscanf(temp,'%f %f');
    % res = 1;
    % fclose(evalin('base','fidin'));
    % evalin('base','fidin = fopen(''mat2mod'',''r'');');
else
    set_param(gcs, 'SimulationCommand', 'stop');
end
outp = res;
end