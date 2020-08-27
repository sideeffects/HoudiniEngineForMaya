from maya import cmds

def get_selected_components(component_type):
    components = []
    sel_mask   = -1
    type_name  = ''
    expand     = False

    lwr_component_type = component_type.lower()

    if lwr_component_type == 'vertex':
        sel_mask  = 31
        type_name = 'vertices'
    elif lwr_component_type == 'edge':
        sel_mask  = 32
        type_name = 'edges'
        expand    = True
    elif lwr_component_type == 'face':
        sel_mask  = 34
        type_name = 'faces'
    elif lwr_component_type == 'uv':
        sel_mask  = 35
        type_name = 'UVs'
    else:
        cmds.error('Unknown component type. Must be "vertex", "edge", "face" or "uv".')

    def extract_component(component):
        lidx = component.find('[') + 1
        ridx = component.rfind(']')

        if lidx <= 0 or ridx <= 0 or ridx <= lidx:
            cmds.warning('Could not determine %s index of "%s"' % (type_name, component))

        return component[lidx:ridx]

    components_raw = cmds.filterExpand(cmds.ls(sl=True), sm=sel_mask, ex=expand)

    if not components_raw:
        return None

    # Edges are handled differently in Houdini. Instead of an edge index, it is
    # specified as two points with a "p" prefix. Eg "p1-3" represents an edge
    # between point 1 and point 3.
    if lwr_component_type == 'edge':
        vertex_components = []

        for component in components_raw:
            raw_vtx_components = cmds.polyListComponentConversion(component, fe=True, tv=True)
            
            if len(raw_vtx_components) == 2:
                vertex_components.append('p' + '-'.join([extract_component(raw_vtx_component) for raw_vtx_component in raw_vtx_components]))
            elif len(raw_vtx_components) == 1:
                vertex_components.append('p' + extract_component(raw_vtx_components[0]).replace(':', '-'))
            else:
                cmds.error('Unable to convert edge with %d vertices to a single range of vertices.' % len(raw_vtx_components))
            
        return ' '.join(vertex_components)

    for component in components_raw:
        components.append(extract_component(component).replace(':', '-'))

    return ' '.join(components)
