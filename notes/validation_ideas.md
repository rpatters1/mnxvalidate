## Validation ideas

- confirm global.lyrics.lineOrder array matches keys in lineMetadata.
- validate voice uniqueness for sequence objects
- validate that event objects either have a duration or a full-measure indication but not both.
- validate that lyric line labels in event objects are valid values.
- events should either be marked rest or have notes.
- event ids and note ids should be unique
- tied-to note ids should have the same pitch name, octave, and alteration as their counterparts.
- events in beams may only be specified once.
- events in beams must have a correct note value for the beam.

- confirm staff systems neither skip nor overlap measures. (done)

### 1.0 completed

- confirm multimeasure rests span measures that exist
- confirm measure counts in each part (optionally: it may be permissible to have different numbers of measures per part. need to check)
- confirm layouts in "scores" have been defined in "layouts". This is required by the MNX doc and not enforced by the schema.
- confirm measures exist in staff systems
- confirm "part" and "labelref" values in staff and staff source objects contain valid values/part references (part ids)
- valid values for enums (some of these are enforced by the schema; we'll need to check):
    - "labelref" (not enforced)
    - "stem" (staff source stem direction) -- enforced by schema
    - "symbol" (group bracket) -- enforced by schema

### Questions and Suggestions

- more symbol styles for groups (in particular, deskBracket)
- since the staff object has a "labelref" property, should it not also have a "part" property?
- staff group objects need a labelref alternative (and "part" that it references).
- labelRef values are not enforced by the schema
- there needs to be a mechanism to specify automatic name/shortName for "labelref" values.
- should "systems" be a required propery of the page object?

