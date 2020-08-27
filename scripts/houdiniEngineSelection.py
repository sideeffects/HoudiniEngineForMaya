from maya import cmds

def get_selected_components(component_type):
    components = []
    sel_mask   = -1
    type_name  = ''

    if component_type.lower() == 'vertex':
        sel_mask = 31
        type_name = 'vertices'
    elif component_type.lower() == 'edge':
        sel_mask = 32
        type_name = 'edges'
    elif component_type.lower() == 'face':
        sel_mask = 34
        type_name = 'faces'
    elif component_type.lower() == 'uv':
        sel_mask = 35
        type_name = 'UVs'
    else:
        cmds.error('Unknown component type. Must be "vertex", "edge", "face" or "uv".')

    components_raw = cmds.filterExpand(cmds.ls(sl=True), sm=sel_mask, ex=False)

    if not components_raw:
        return None

    for component in components_raw:
        lidx = component.find('[') + 1
        ridx = component.rfind(']')

        if lidx <= 0 or ridx <= 0 or ridx <= lidx:
            cmds.warning('Could not determine %s index of "%s"' % (type_name, component))

        components.append(component[lidx:ridx].replace(':', '-'))

    return ', '.join(components)

